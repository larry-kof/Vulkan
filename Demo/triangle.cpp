

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
    { { -1.0,-0.5 } },
    { { 0.0,0.5 } },
    { { 1.0,-0.5 } },
//    { { -0.5f, 0.5f } },
//    { { -0.5f, -0.5f } },
//    { { 0.5f, 0.5f } },
    //    { { -0.5f, -0.5f } },
    //    { { 0.5f, 0.5f } },
    //    { { 0.5f, -0.5f } }
};

const static std::vector<unsigned short> indices = {
    0,1,2
};

static std::shared_ptr<VksBuffer> generateIndiceBuffer()
{
    auto indicesStagingBuffer = VksBuffer::createBuffer( sizeof( indices[0] ) * indices.size() , VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
    indicesStagingBuffer->copyHostDataToBuffer( (void*)indices.data(), sizeof( indices[0] ) * indices.size() );
    
    auto indicesBuffer = VksBuffer::createBuffer(sizeof( indices[0] ) * indices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    indicesBuffer->copyFromBuffer( *indicesStagingBuffer );
    
    return indicesBuffer;
}

int triangleDemo( VksSwapChain& swapChain )
{
    try{

        std::shared_ptr<VksShaderProgram> shaderProgram( new VksShaderProgram( std::string("shaders/simpleVert.spv"),
                                                                              std::string("shaders/simpleFrag.spv") ) );
        
        std::vector<VksShaderProgram::UniformLayoutBinding> layoutBindings;
        std::vector<VksShaderProgram::DescriptorPoolInfo> descPools;
         
        descPools.push_back( VksShaderProgram::DescriptorPoolInfo( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapChain.getSwapChainCount() ) );
        layoutBindings.push_back( { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT } );
         
        shaderProgram->initialize(layoutBindings, descPools, swapChain.getSwapChainCount());
        
        auto uniformBuffer = VksBuffer::createBuffer(sizeof( glm::mat4 ), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        glm::mat4 model = glm::identity<glm::mat4>();
        model = glm::scale(model, glm::vec3( 1.0,-1.0,1.0 ));
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), swapChain.getRenderAreaSize().width / (float) swapChain.getRenderAreaSize().height, 0.1f, 10.0f);

        glm::mat4 MVP = proj * view * model;
        
        VkExtent2D screenSize = swapChain.getRenderAreaSize();
        
        uniformBuffer->copyHostDataToBuffer( glm::value_ptr( MVP ) , sizeof(glm::mat4));
        
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
        
        auto indicesBuffer = generateIndiceBuffer();
        
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
            frameBuffer->bindIndexBuffer( indicesBuffer );
            frameBuffer->bindUniformSets( i );
            frameBuffer->drawIndexed( 3 );
            frameBuffer->unBind();
        }
        
        auto callback = [ uniformBuffer,&screenSize]( int msecs )
        {
            static float currentAngle = 0;
            glm::mat4 model = glm::identity<glm::mat4>();
            model = glm::scale(model, glm::vec3( 1.0,-1.0,1.0 ));
            model = glm::rotate(model, glm::radians( currentAngle ), glm::vec3(0.0,0.0,1.0));

            glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)screenSize.width / (float)screenSize.height, 0.1f, 10.0f);
            glm::mat4 MVP = proj * view * model;
            currentAngle += (float)msecs / 1000 * 10;
            
            uniformBuffer->copyHostDataToBuffer( glm::value_ptr( MVP ) , sizeof(glm::mat4));
            
        };
        
        swapChain.drawFrames( callback );
        
    }catch( const std::exception& e )
    {
        std::cout << " exception = " << e.what();
    }
    return 0;
}
