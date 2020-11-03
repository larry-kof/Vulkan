//
//  depth_example.cpp
//  Vulkan
//
//  Created by CL10097-TMP on 2020/10/20.
//  Copyright © 2020 larry. All rights reserved.
//

#include <stdio.h>
#include "VksSwapChain.hpp"
#include "VksShaderProgram.hpp"
#include "VksAttribute.hpp"
#include "VksGraphicPipeline.hpp"
#include "VksColorBlend.hpp"
#include "VksDepthStencil.hpp"
#include "VksCommand.hpp"
#include <memory>
#include <string>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
struct DepthVertex {
    glm::vec3 pos;
};

const static std::vector<DepthVertex> vertices = {
    { { -0.5f, 0.5f, 0.0f } },
    { { -0.5f, -0.5f, 0.0f } },
    { { 0.5f, 0.5f, 0.0f } },
    
    { { -0.5f, 0.5f, -0.5f } },
    { { -0.5f, -0.5f, -0.5f } },
    { { 0.5f, 0.5f, -0.5f } },
    
};

int depthDemo( VksSwapChain& swapChain )
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
        

        glm::mat4 model = glm::identity<glm::mat4>();
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), swapChain.getRenderAreaSize().width / (float) swapChain.getRenderAreaSize().height, 0.1f, 10.0f);

        proj[1][1] *= -1.0f;
        glm::mat4 MVP = proj * view * model;

        uniformBuffer->copyHostDataToBuffer( glm::value_ptr( MVP ) , sizeof(glm::mat4));
        
        for(int i = 0; i < swapChain.getSwapChainCount(); i++)
            shaderProgram->updateShaderUniform(i, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, *uniformBuffer);
        
        std::vector<VkVertexInputAttributeDescription> inputAttriDescs;
        VkVertexInputAttributeDescription attributeDesc = {};
        attributeDesc.binding = 0;
        attributeDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDesc.location = 0;
        attributeDesc.offset = 0;
        inputAttriDescs.push_back( attributeDesc );
        auto attribute = std::make_shared<VksAttribute>( sizeof(DepthVertex), inputAttriDescs );
        attribute->setTopology( VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST );
        attribute->createVertexBuffer( reinterpret_cast<const void*>( vertices.data() ), static_cast<int>( sizeof( vertices[0] ) * vertices.size() ) );
        
        auto graphicPipeline = std::make_shared<VksGraphicPipeline>();
        graphicPipeline->addComponent<VksShaderProgram>( shaderProgram.get() );
        graphicPipeline->addComponent<VksAttribute>(attribute.get());
        
        auto colorBlend = std::make_shared<VksColorBlend>();
        auto depthStencil = std::make_shared<VksDepthStencil>();
        depthStencil->enableDepthTest();
        
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
