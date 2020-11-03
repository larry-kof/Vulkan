

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

struct TriangleVertex {
    glm::vec2 pos;
};

const static std::vector<TriangleVertex> vertices = {
    { { -0.5f, 0.5f } },
    { { -0.5f, -0.5f } },
    { { 0.5f, 0.5f } },
    //    { { -0.5f, -0.5f } },
    //    { { 0.5f, 0.5f } },
    //    { { 0.5f, -0.5f } }
};

int triangleDemo( VksSwapChain& swapChain )
{
    try{

        std::shared_ptr<VksShaderProgram> shaderProgram( new VksShaderProgram( std::string("shaders/simpleVert.spv"),
                                                                              std::string("shaders/simpleFrag.spv") ) );
        
        std::vector<VksShaderProgram::SetLayoutBinding> layoutBindings;
        std::vector<VksShaderProgram::DescriptorPool> descPools;
        
        descPools.push_back( std::make_tuple( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapChain.getSwapChainCount() ) );
        layoutBindings.push_back( std::make_tuple( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT ) );
        
        shaderProgram->initialize(layoutBindings, descPools, swapChain.getSwapChainCount());
        
        auto uniformBuffer = VksBuffer::createBuffer(sizeof( glm::mat4 ), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        glm::mat4 proj = glm::identity<glm::mat4>();
        proj[1][1] *= -1.0f;
        
        uniformBuffer->copyHostDataToBuffer( glm::value_ptr( proj ) , sizeof(glm::mat4));
        
        for(int i = 0; i < swapChain.getSwapChainCount(); i++)
            shaderProgram->updateShaderUniform(i, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, *uniformBuffer);
        
        std::vector<VkVertexInputAttributeDescription> inputAttriDescs;
        VkVertexInputAttributeDescription attributeDesc = {};
        attributeDesc.binding = 0;
        attributeDesc.format = VK_FORMAT_R32G32_SFLOAT;
        attributeDesc.location = 0;
        attributeDesc.offset = 0;
        inputAttriDescs.push_back( attributeDesc );
        auto attribute = std::make_shared<VksAttribute>( sizeof(TriangleVertex), inputAttriDescs );
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
