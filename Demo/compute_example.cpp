
#include "VksSwapChain.hpp"
#include "VksShaderProgram.hpp"
#include "VksAttribute.hpp"
#include "VksGraphicPipeline.hpp"
#include "VksColorBlend.hpp"
#include "VksDepthStencil.hpp"
#include "VksCommand.hpp"
#include "VksCompute.hpp"
#include "VksBarrier.hpp"
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

static struct TextureVertex {
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
    //    { { -0.5f, -0.5f } },
    //    { { 0.5f, 0.5f } },
    //    { { 0.5f, -0.5f } }
};

static std::shared_ptr<VksCompute<> > prepareCompute()
{
    auto computeShader = std::make_shared<VksShaderProgram>( std::string( "shaders/edgedetect.comp.spv" ) );
    
    std::vector<VksShaderProgram::DescriptorPoolInfo> descPools;
    std::vector<VksShaderProgram::UniformLayoutBinding> layoutBindings;
    
    descPools.push_back( VksShaderProgram::DescriptorPoolInfo( VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2 ));
    
    layoutBindings.push_back( VksShaderProgram::UniformLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) );
    layoutBindings.push_back( VksShaderProgram::UniformLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) );
    
    computeShader->initialize(layoutBindings, descPools, 1);
    
    auto inputTexture = VksTexture::createFromFile("texture1.jpg", VK_IMAGE_USAGE_STORAGE_BIT);
    auto outputTexture = VksTexture::createEmptyTexture(inputTexture->getWidth(), inputTexture->getHeight(), inputTexture->getFormat(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    
    computeShader->updateSampler(0, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, *inputTexture);
    computeShader->updateSampler(0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, *outputTexture);
    
    auto compute = std::make_shared<VksCompute<> >( computeShader );
    
    compute->setComputeInputOutput(inputTexture, outputTexture);
    compute->prepareCompute(inputTexture->getWidth(), 16, inputTexture->getHeight(), 16);
    
    return compute;
}

int computeDemo( VksSwapChain& swapChain )
{
    try{
        std::shared_ptr<VksShaderProgram> shaderProgram( new VksShaderProgram( std::string("shaders/textureVert.spv"),
                                                                              std::string("shaders/textureFrag.spv") ) );
        
        std::vector<VksShaderProgram::DescriptorPoolInfo> descPools;
        std::vector<VksShaderProgram::UniformLayoutBinding> layoutBindings;
        
        descPools.push_back( VksShaderProgram::DescriptorPoolInfo( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapChain.getSwapChainCount() ) );
        descPools.push_back( VksShaderProgram::DescriptorPoolInfo( VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, swapChain.getSwapChainCount() ) );
        layoutBindings.push_back( VksShaderProgram::UniformLayoutBinding( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT ) );
        layoutBindings.push_back( VksShaderProgram::UniformLayoutBinding( 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT ) );
        
        shaderProgram->initialize(layoutBindings, descPools, swapChain.getSwapChainCount());
        
        auto uniformBuffer = VksBuffer::createBuffer(sizeof( glm::mat4 ), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        glm::mat4 proj = glm::identity<glm::mat4>();
        proj[1][1] *= -1.0f;
        
        uniformBuffer->copyHostDataToBuffer( glm::value_ptr( proj ) , sizeof(glm::mat4));
        
        auto edgeCompute = prepareCompute();
        
        for(int i = 0; i < swapChain.getSwapChainCount(); i++)
        {
            shaderProgram->updateShaderUniform(i, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, *uniformBuffer);
//            shaderProgram->updateSampler(i, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, *texturePtr);
            shaderProgram->updateSampler(i, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, *(edgeCompute->getOutput()) );
        }
        
        std::vector<VkVertexInputAttributeDescription> inputAttriDescs {
            { 0,0,VK_FORMAT_R32G32_SFLOAT,0 },
            { 1,0,VK_FORMAT_R32G32_SFLOAT, offsetof(TextureVertex, texCoord)}
        };
        auto attribute = std::make_shared<VksAttribute>( sizeof(TextureVertex), inputAttriDescs );
        attribute->setTopology( VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST );
        attribute->createVertexBuffer( reinterpret_cast<const void*>( vertices.data() ), static_cast<int>( sizeof( vertices[0] ) * vertices.size() ) );
        
        auto graphicPipeline = std::make_shared<VksGraphicPipeline>();
        graphicPipeline->addComponent<VksShaderProgram>( shaderProgram.get() );
        graphicPipeline->addComponent<VksAttribute>(attribute.get());
        
        auto colorBlend = std::make_shared<VksColorBlend>();
        auto depthStencil = std::make_shared<VksDepthStencil>();
        
        graphicPipeline->addComponent<VksColorBlend>(colorBlend.get());
        graphicPipeline->addComponent<VksDepthStencil>(depthStencil.get());
        
        int swapChainCount = swapChain.getSwapChainCount();

        for(int i = 0; i < swapChainCount; i++)
        {
            auto frameBuffer = swapChain.getSwapChainFrameBuffer( i );
            frameBuffer->bind();
            frameBuffer->useGraphicPipeline( graphicPipeline );
            frameBuffer->bindVertexBuffer(attribute->getVertexBuffer());
            frameBuffer->bindUniformSets( i );
            frameBuffer->draw( 6 );
            frameBuffer->unBind();
        }
        
        auto func = [edgeCompute]( const std::vector<VkSemaphore>& waitSemas, std::vector<VkSemaphore>& signalSemas,
                                 std::vector<VkPipelineStageFlags>& expectedStages)
        {
            edgeCompute->submitWork(waitSemas, signalSemas);
            expectedStages.clear();
        };
        
        auto funcDrawTime = [edgeCompute]( int timeStamp ){
            static int totalTime = 0;
            static bool pic = true;
            if( totalTime > 2000 )
            {
                std::shared_ptr<VksTexture> inputTexture = edgeCompute->getInput();
                inputTexture->updateTexture( pic ? "texture.jpg" : "texture1.jpg");
                pic = !pic;
                totalTime -= 2000;
            }
            totalTime += timeStamp;
        };
        
        swapChain.drawFrames( func, funcDrawTime );
        
    }catch( const std::exception& e )
    {
        std::cout << " exception = " << e.what();
    }
    return 0;
}
