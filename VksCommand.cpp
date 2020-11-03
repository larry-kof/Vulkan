#include "VksCommand.hpp"
#include <array>

std::shared_ptr<VksCommand> VksCommand::createCommandPool(uint32_t queueFamilyIndex)
{
    VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = queueFamilyIndex
    };

    VkCommandPool cPool;
    VK_CHECK( vkCreateCommandPool(m_logicDevice, &info, nullptr, &cPool) )

    return  std::shared_ptr<VksCommand>(new VksCommand( cPool ) );
}

VksCommand::VksCommand( VkCommandPool commandPool  )
    :m_commandPool( commandPool )
{
}

VksCommand::~VksCommand()
{
    vkDestroyCommandPool( m_logicDevice, m_commandPool, nullptr );
}

VkCommandBuffer VksCommand::beginOnceSubmitBuffer()
{
    VkCommandBufferAllocateInfo bufferAllocInfo = {};
    bufferAllocInfo.commandBufferCount = 1;
    bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bufferAllocInfo.commandPool = m_commandPool;
    bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    VkCommandBuffer commanbuffer;
    vkAllocateCommandBuffers(m_logicDevice, &bufferAllocInfo, &commanbuffer);
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer( commanbuffer, &beginInfo );

    return commanbuffer;
}

void VksCommand::endOnceSubmitBuffer( VkCommandBuffer commandbuffer )
{
    vkEndCommandBuffer( commandbuffer );
    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &commandbuffer;

    vkQueueSubmit( m_graphicsQueue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle( m_graphicsQueue );

    vkFreeCommandBuffers(m_logicDevice, m_commandPool, 1, &commandbuffer);
}

std::vector<VkCommandBuffer> VksCommand::createPrimaryBuffer(int size)
{
    if( size <= 0 ) return std::vector<VkCommandBuffer>();
    std::vector<VkCommandBuffer> buffers(size);
    VkCommandBufferAllocateInfo commandBufferInfo = {};
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.commandPool = m_commandPool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = static_cast<uint32_t>( buffers.size() );
    
    VK_CHECK( vkAllocateCommandBuffers(m_logicDevice, &commandBufferInfo, buffers.data()) )

    return buffers;
}

VkCommandBuffer VksCommand::createPrimaryBuffer()
{
    VkCommandBufferAllocateInfo commandBufferInfo = {};
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.commandPool = m_commandPool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = 1;
    
    VkCommandBuffer commanBuffer;
    
    VK_CHECK( vkAllocateCommandBuffers(m_logicDevice, &commandBufferInfo, &commanBuffer) )
    
    return commanBuffer;
}

VkCommandPool VksCommand::getCommandPool() const
{
    return m_commandPool;
}

void VksCommand::beginRenderPass( VkCommandBuffer commandBuffer, const std::shared_ptr<VksFramebuffer> dstFramebuffer)
{
    VkCommandBufferBeginInfo bufferBeginInfo = {};
    bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    bufferBeginInfo.pInheritanceInfo = nullptr;
    
    VK_CHECK( vkBeginCommandBuffer(commandBuffer, &bufferBeginInfo) )
    
    
    VkRenderPassBeginInfo renderBeginInfo = {};
    renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    
    renderBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderBeginInfo.pClearValues = clearValues.data();
    
    VkExtent2D renderSize = dstFramebuffer->getFramebufferSize();
    renderBeginInfo.framebuffer = dstFramebuffer->getVkFramebuffer();
    renderBeginInfo.renderArea.offset = {0,0};
    renderBeginInfo.renderArea.extent = renderSize;
    renderBeginInfo.renderPass = dstFramebuffer->getVkRenderPass();
    
    vkCmdBeginRenderPass(commandBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    VkViewport viewport = {  };
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = renderSize.width;
    viewport.height = renderSize.height;
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;
    
    VkViewport viewports[] = { viewport };
    
    vkCmdSetViewport(commandBuffer, 0, 1, viewports);
    
    VkRect2D scissor = { {0,0}, renderSize };
    VkRect2D scissors[] = { scissor };
    
    vkCmdSetScissor(commandBuffer, 0, 1, scissors);
}

void VksCommand::endRenderPass(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
    
    VK_CHECK( vkEndCommandBuffer( commandBuffer ) )
}

void VksCommand::bindVertexBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<VksBuffer> vertexBuffer)
{
    VkBuffer vkBuffer = vertexBuffer->getVkBuffer();
    VkBuffer vertexBuffers[] = { vkBuffer };
    VkDeviceSize offsets[] = {0};
    
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
}

void VksCommand::bindIndexBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<VksBuffer> indexBuffer, VkIndexType indexValType)
{
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getVkBuffer(), 0, indexValType);
}

void VksCommand::drawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount)
{
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void VksCommand::draw(VkCommandBuffer commandBuffer, uint32_t vertexCount)
{
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}

void VksCommand::bindUniformSet(VkCommandBuffer commandBuffer, const std::shared_ptr<VksShaderProgram> &shaderPtr, VkDescriptorSet descriptorSet)
{
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPtr->getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
}
