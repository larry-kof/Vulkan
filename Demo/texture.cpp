
#include "VksSwapChain.hpp"
#include "VksShaderProgram.hpp"
#include "VksAttribute.hpp"
#include "VksGraphicPipeline.hpp"
#include "VksColorBlend.hpp"
#include "VksDepthStencil.hpp"
#include "VksCommand.hpp"
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

struct TextureVertex {
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

int textureDemo( VksSwapChain& swapChain )
{
    try{
        std::shared_ptr<VksShaderProgram> shaderProgram( new VksShaderProgram( std::string("shaders/textureVert.spv"),
                                                                              std::string("shaders/textureFrag.spv") ) );
        
        std::vector<VksShaderProgram::SetLayoutBinding> layoutBindings;
        std::vector<VksShaderProgram::DescriptorPool> descPools;
        
        descPools.push_back( std::make_tuple( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapChain.getSwapChainCount() ) );
        descPools.push_back( std::make_tuple( VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, swapChain.getSwapChainCount() ) );
        layoutBindings.push_back( std::make_tuple( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT ) );
        layoutBindings.push_back( std::make_tuple( 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) );
        
        shaderProgram->initialize(layoutBindings, descPools, swapChain.getSwapChainCount());
        
        auto uniformBuffer = VksBuffer::createBuffer(sizeof( glm::mat4 ), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        glm::mat4 proj = glm::identity<glm::mat4>();
        proj[1][1] *= -1.0f;
        
        uniformBuffer->copyHostDataToBuffer( glm::value_ptr( proj ) , sizeof(glm::mat4));
        
        auto texturePtr = VksTexture::createFromFile("texture.jpg", VK_IMAGE_USAGE_SAMPLED_BIT);
        
        for(int i = 0; i < swapChain.getSwapChainCount(); i++)
        {
            shaderProgram->updateShaderUniform(i, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, *uniformBuffer);
            shaderProgram->updateSampler(i, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, *texturePtr);
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
        
        swapChain.drawFrames();
        
    }catch( const std::exception& e )
    {
        std::cout << " exception = " << e.what();
    }
    return 0;
}
