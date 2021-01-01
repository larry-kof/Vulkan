
#include "VksSwapChain.hpp"
#include "VksShaderProgram.hpp"
#include "VksAttribute.hpp"
#include "VksGraphicPipeline.hpp"
#include "VksColorBlend.hpp"
#include "VksDepthStencil.hpp"
#include "VksCommand.hpp"
#include "VksCompute.hpp"
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

struct TextureVertex
{
    glm::vec2 pos;
    glm::vec2 texCoord;
};

const static std::vector<TextureVertex> vertices = {
    { { -0.5f, 0.5f }, { 0.0f, 1.0f } },
    { { -0.5f, -0.5f }, { 0.0f, 0.0f } },
    { { 0.5f, 0.5f }, { 1.0f, 1.0f } },

    { { 0.5f, 0.5f }, { 1.0f, 1.0f } },
    { { -0.5f, -0.5f }, { 0.0f, 0.0f } },
    { { 0.5f, -0.5f }, { 1.0f, 0.0f } },
};

const static std::vector<TextureVertex> offScreenVertices = {
    { { -1.0f, 1.0f }, { 0.0f, 1.0f } },
    { { -1.0f, -1.0f }, { 0.0f, 0.0f } },
    { { 1.0f, 1.0f }, { 1.0f, 1.0f } },

    { { 1.0f, 1.0f }, { 1.0f, 1.0f } },
    { { -1.0f, -1.0f }, { 0.0f, 0.0f } },
    { { 1.0f, -1.0f }, { 1.0f, 0.0f } },
};

struct OffScreen
{
    std::shared_ptr<VksTexture> inputTexture;
    std::shared_ptr<VksFramebuffer> offscreenBuffer;
    std::shared_ptr<VksGraphicPipeline> offPipeline;
    std::shared_ptr<VksBuffer> uniformBuffer;
};

static void genOffPipeline( std::shared_ptr<OffScreen> offscreen )
{
    std::shared_ptr<VksShaderProgram> shader(
        new VksShaderProgram( "shaders/offscreenVert.spv", "shaders/offscreenFrag.spv" ) );

    std::vector<VksShaderProgram::DescriptorPoolInfo> descInfos;
    std::vector<VksShaderProgram::UniformLayoutBinding> uniformBindings;

    descInfos.push_back( VksShaderProgram::DescriptorPoolInfo( VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 ) );
    descInfos.push_back( VksShaderProgram::DescriptorPoolInfo( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 ) );

    uniformBindings.push_back( { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT } );
    uniformBindings.push_back( { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT } );

    shader->initialize( uniformBindings, descInfos, 1 );

    glm::mat4 proj = glm::identity<glm::mat4>();
    proj[1][1] *= -1.0f;

    auto uniformBuffer =
        VksBuffer::createBuffer( sizeof( glm::mat4 ),
                                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
    uniformBuffer->copyHostDataToBuffer( glm::value_ptr(proj), sizeof(glm::mat4), 0 );
    offscreen->uniformBuffer = uniformBuffer;
    shader->updateShaderUniform( 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, *uniformBuffer );

    shader->updateSampler( 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, *(offscreen->inputTexture));

    std::vector<VkVertexInputAttributeDescription> inputAttriDescs = {
        { 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 },
        { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( TextureVertex, texCoord ) },
    };
    auto attribute = std::make_shared<VksAttribute>( sizeof( TextureVertex ), inputAttriDescs );
    attribute->setTopology( VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST );
    attribute->createVertexBuffer( reinterpret_cast<const void*>( offScreenVertices.data() ),
                                       static_cast<int>( sizeof( offScreenVertices[0] ) * offScreenVertices.size() ) );
    
    auto graphicPipeline = std::make_shared<VksGraphicPipeline>();
    graphicPipeline->addComponent<VksShaderProgram>( shader.get() );
    graphicPipeline->addComponent<VksAttribute>( attribute.get() );

    offscreen->offPipeline = graphicPipeline;

    offscreen->offscreenBuffer->bind();
    offscreen->offscreenBuffer->useGraphicPipeline( graphicPipeline );
    offscreen->offscreenBuffer->bindVertexBuffer( attribute->getVertexBuffer() );
    offscreen->offscreenBuffer->bindUniformSets( 0 );
    offscreen->offscreenBuffer->draw( 6 );
    offscreen->offscreenBuffer->unBind();

    std::vector<VkSemaphore> waitSemas;
    std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT};
    std::vector<VkSemaphore> signalSemas;
    offscreen->offscreenBuffer->submitRender( waitSemas, waitStages, signalSemas );
}

static std::shared_ptr<OffScreen> offscreenRender( int width, int height, VkFormat format )
{
    auto offsreen = std::make_shared<OffScreen>();

    auto colorTexture =
        VksTexture::createEmptyTexture( width,
                                        height,
                                        format,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                        VK_IMAGE_ASPECT_COLOR_BIT );

    auto renderPass = std::make_shared<VksRenderPass>();
    renderPass->addColorAttachment( format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkSubpassDependency> dependencies( 2 );

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    renderPass->addSubpassDependency( dependencies );

    renderPass->createRenderPass();

    auto framebuffer = std::make_shared<VksFramebuffer>( colorTexture, renderPass );
    offsreen->offscreenBuffer = framebuffer;

    auto inputTexture = VksTexture::createFromFile(
        "texture.jpg", VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

    offsreen->inputTexture = inputTexture;

    genOffPipeline( offsreen );

    return offsreen;
}

int textureDemo( VksSwapChain& swapChain )
{
    try
    {
        auto offscreen = offscreenRender( swapChain.getRenderAreaSize().width, swapChain.getRenderAreaSize().height,
                    VK_FORMAT_R8G8B8A8_UNORM );
        std::shared_ptr<VksShaderProgram> shaderProgram( new VksShaderProgram(
            std::string( "shaders/textureVert.spv" ), std::string( "shaders/textureFrag.spv" ) ) );

        std::vector<VksShaderProgram::DescriptorPoolInfo> descPools;
        std::vector<VksShaderProgram::UniformLayoutBinding> layoutBindings;

        descPools.push_back(
            VksShaderProgram::DescriptorPoolInfo( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapChain.getSwapChainCount() ) );
        descPools.push_back( VksShaderProgram::DescriptorPoolInfo( VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                                   swapChain.getSwapChainCount() ) );
        layoutBindings.push_back( VksShaderProgram::UniformLayoutBinding(
            0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT ) );
        layoutBindings.push_back( VksShaderProgram::UniformLayoutBinding(
            1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT ) );

        shaderProgram->initialize( layoutBindings, descPools, swapChain.getSwapChainCount() );

        auto uniformBuffer =
            VksBuffer::createBuffer( sizeof( glm::mat4 ),
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

        glm::mat4 proj = glm::identity<glm::mat4>();
        proj[1][1] *= -1.0f;

        uniformBuffer->copyHostDataToBuffer( glm::value_ptr( proj ), sizeof( glm::mat4 ) );

        // auto texturePtr = VksTexture::createFromFile( "texture.jpg", VK_IMAGE_USAGE_SAMPLED_BIT );

        for( int i = 0; i < swapChain.getSwapChainCount(); i++ )
        {
            shaderProgram->updateShaderUniform( i, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, *uniformBuffer );
            // shaderProgram->updateSampler( i, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, *texturePtr );
            shaderProgram->updateSampler( i, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, *( offscreen->offscreenBuffer->getColorTexture() ) );
        }

        std::vector<VkVertexInputAttributeDescription> inputAttriDescs{
            { 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 },
            { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( TextureVertex, texCoord ) } };
        auto attribute = std::make_shared<VksAttribute>( sizeof( TextureVertex ), inputAttriDescs );
        attribute->setTopology( VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST );
        attribute->createVertexBuffer( reinterpret_cast<const void*>( vertices.data() ),
                                       static_cast<int>( sizeof( vertices[0] ) * vertices.size() ) );

        auto graphicPipeline = std::make_shared<VksGraphicPipeline>();
        graphicPipeline->addComponent<VksShaderProgram>( shaderProgram.get() );
        graphicPipeline->addComponent<VksAttribute>( attribute.get() );

        auto colorBlend = std::make_shared<VksColorBlend>();
        auto depthStencil = std::make_shared<VksDepthStencil>();

        graphicPipeline->addComponent<VksColorBlend>( colorBlend.get() );
        graphicPipeline->addComponent<VksDepthStencil>( depthStencil.get() );

        int swapChainCount = swapChain.getSwapChainCount();

        for( int i = 0; i < swapChainCount; i++ )
        {
            auto frameBuffer = swapChain.getSwapChainFrameBuffer( i );
            frameBuffer->bind();
            frameBuffer->useGraphicPipeline( graphicPipeline );
            frameBuffer->bindVertexBuffer( attribute->getVertexBuffer() );
            frameBuffer->bindUniformSets( i );
            frameBuffer->draw( 6 );
            frameBuffer->unBind();
        }


        auto submitWork = [offscreen]( const std::vector<VkSemaphore>& waitSemas, std::vector<VkSemaphore>& signalSemas,std::vector<VkPipelineStageFlags>& nextStage )
        {
            // std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT};
            // offscreen->offscreenBuffer->submitRender( waitSemas, waitStages, signalSemas );
            // nextStage.clear();
            // nextStage.push_back( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT );
        };
        swapChain.drawFrames( submitWork, []( int msec ) {} );

    }
    catch( const std::exception& e )
    {
        std::cout << " exception = " << e.what();
    }
    return 0;
}

int main()
{
    VksSwapChain swapChain;
    return textureDemo( swapChain );
}