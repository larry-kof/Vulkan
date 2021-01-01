//
//  VksCompute.hpp
//  Vulkan
//
//  Created by larry-kof on 2020/11/13.
//  Copyright © 2020 larry. All rights reserved.
//

#ifndef VksCompute_hpp
#define VksCompute_hpp

#include "VkEngine.hpp"
#include "VksShaderProgram.hpp"
#include "VksCommand.hpp"
#include <cmath>
#include <type_traits>

class VksTexture;
class VksBuffer;

template <typename T, typename Enable=void>
struct check;
 
template <typename T>
struct check<T, typename std::enable_if<T::value>::type> {
    static constexpr bool value = T::value;
    using type = T;
};

template<class INPUT = VksTexture, class OUTPUT = VksTexture>
class VksCompute : protected VkEngine
{
public:
    
    using IN_TYPE = typename std::enable_if< check<INPUT>::value, INPUT >::type;
    using OUT_TYPE = typename std::enable_if< check<OUTPUT>::value, OUTPUT >::type;

    VksCompute( const std::shared_ptr<VksShaderProgram>& shader )
    :m_computePipeline( VK_NULL_HANDLE ), m_commandBuffer( VK_NULL_HANDLE )
    ,m_computeShader( shader ), m_computeComplete( VK_NULL_HANDLE )
    {
        m_commandBuffer = m_graphicCommand->createPrimaryBuffer();
        __createComputePipeline();
        
        VkSemaphoreCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        vkCreateSemaphore(m_logicDevice, &createInfo, nullptr, &m_computeComplete);
        
    }
    
    ~VksCompute()
    {
        if( m_commandBuffer )
        {
            vkFreeCommandBuffers(m_logicDevice, m_graphicCommand->getCommandPool(), 1, &m_commandBuffer);
        }
        
        if( m_computePipeline )
        {
            vkDestroyPipeline(m_logicDevice, m_computePipeline, nullptr);
        }
        
        if( m_computeComplete )
        {
            vkDestroySemaphore(m_logicDevice, m_computeComplete, nullptr);
        }
    }
    
    void prepareCompute( int globalWidth, int groupWidth, int globalHeight, int groupHeight,
                         int globalDepth = -1, int groupDepth = -1 )
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        
        VkPipelineLayout layout = m_computeShader->getPipelineLayout();
        VkDescriptorSet descSet = m_computeShader->getDescriptorSet();
        
        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
     
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
        vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, layout, 0, 1, &descSet, 0, nullptr);
        
        vkCmdDispatch( m_commandBuffer, (uint32_t)ceil( globalWidth / (float)groupWidth ),
                      (uint32_t)ceil( globalHeight / (float)groupHeight ),
                      (uint32_t)ceil( globalDepth / (float)groupDepth ) );
        vkEndCommandBuffer(m_commandBuffer);
    }
    
    void setComputeInputOutput( std::shared_ptr<IN_TYPE> input, std::shared_ptr<OUT_TYPE> output )
    {
        m_input = input;
        m_output = output;
    }
    
    void submitWork( const std::vector<VkSemaphore>& waitSemaphores, std::vector<VkSemaphore>& signalSemaphores )
    {
        VkPipelineStageFlags waitDstStages[] = { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT  };
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffer;
        submitInfo.pWaitDstStageMask = waitDstStages;
        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_computeComplete;
        vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        
        signalSemaphores.clear();
        signalSemaphores.push_back( m_computeComplete );
        
//        vkQueueWaitIdle(m_graphicsQueue);
    }
    
    std::shared_ptr<OUT_TYPE> getOutput()
    {
        return m_output;
    }
    
    std::shared_ptr<IN_TYPE> getInput()
    {
        return m_input;
    }
    
    VkCommandBuffer getCommandBuffer()
    {
        return m_commandBuffer;
    }
    
protected:
private:
    VkPipeline m_computePipeline;
    VkCommandBuffer m_commandBuffer;
    std::shared_ptr<VksShaderProgram> m_computeShader;
    std::shared_ptr<IN_TYPE> m_input;
    std::shared_ptr<OUT_TYPE> m_output;
    VkSemaphore m_computeComplete;

    void __createComputePipeline()
    {
        VkComputePipelineCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        createInfo.flags = 0;
        createInfo.layout = m_computeShader->getPipelineLayout();
        auto& shaderStages = m_computeShader->getShaderStageCreateInfo();
        createInfo.stage = shaderStages[0];
        createInfo.pNext = nullptr;
        
        VK_CHECK( vkCreateComputePipelines(m_logicDevice, nullptr, 1, &createInfo, nullptr, &m_computePipeline) );
    }
};

#endif /* VksCompute_hpp */
