//
//  VksBarrier.cpp
//  Vulkan
//
//  Created by CL10097-TMP on 2020/11/24.
//  Copyright © 2020 larry. All rights reserved.
//

#include "VksBarrier.hpp"
#include "VksTexture.hpp"
#include "VksBuffer.hpp"

VksBarrier::VksBarrier()
{
    
}
std::shared_ptr<VksBarrier> VksBarrier::createImageBarrier( const std::shared_ptr<VksTexture>& srcImage,
                                                            VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                           VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage )
{

    std::shared_ptr<VksBarrier> barrierPtr( new VksBarrier );
    
    barrierPtr->__addImageBarrier(srcImage, srcAccessMask, dstAccessMask);

    barrierPtr->m_srcStage = srcStage;
    barrierPtr->m_dstStage = dstStage;

    return barrierPtr;
}

std::shared_ptr<VksBarrier> VksBarrier::createBufferBarrier(const std::shared_ptr<VksBuffer> &srcBuffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
{
    std::shared_ptr<VksBarrier> barrierPtr( new VksBarrier );
    
    barrierPtr->__addBufferBarrier(srcBuffer, srcAccessMask, dstAccessMask);

    barrierPtr->m_srcStage = srcStage;
    barrierPtr->m_dstStage = dstStage;

    return barrierPtr;
}

std::shared_ptr<VksBarrier> VksBarrier::createMemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
{
    std::shared_ptr<VksBarrier> barrierPtr( new VksBarrier );
    
    barrierPtr->__addMemoryBarrier(srcAccessMask, dstAccessMask);

    barrierPtr->m_srcStage = srcStage;
    barrierPtr->m_dstStage = dstStage;

    return barrierPtr;
}

void VksBarrier::addImageBarrier( const std::shared_ptr<VksTexture>& srcImage,
                     VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask )
{
    __addImageBarrier(srcImage, srcAccessMask, dstAccessMask);
}

void VksBarrier::addBufferBarrier( const std::shared_ptr<VksBuffer>& srcBuffer,
                      VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask )
{
    __addBufferBarrier(srcBuffer, srcAccessMask, dstAccessMask);
}

void VksBarrier::addMemoryBarrier( VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask  )
{
    __addMemoryBarrier(srcAccessMask, dstAccessMask);
}

void VksBarrier::__addImageBarrier(const std::shared_ptr<VksTexture> &srcImage, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcAccessMask = srcAccessMask;
    imageMemoryBarrier.dstAccessMask = dstAccessMask;
    
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
    imageMemoryBarrier.oldLayout = srcImage->getDesscriptor().imageLayout;
    imageMemoryBarrier.newLayout = srcImage->getDesscriptor().imageLayout;
    imageMemoryBarrier.image = srcImage->getImage();
    
    imageMemoryBarrier.subresourceRange = srcImage->getSubresourceRange();
    
    m_imageBarriers.push_back( imageMemoryBarrier );
}

void VksBarrier::__addBufferBarrier(const std::shared_ptr<VksBuffer>& srcBuffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
    VkBufferMemoryBarrier bufferBarrier = {};
    bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bufferBarrier.srcAccessMask = srcAccessMask;
    bufferBarrier.dstAccessMask = dstAccessMask;
    bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.offset = 0;
    bufferBarrier.size = srcBuffer->getVkBufferSize();
    bufferBarrier.buffer = srcBuffer->getVkBuffer();
    
    m_bufferBarriers.push_back( bufferBarrier );
}

void VksBarrier::__addMemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
    VkMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask = srcAccessMask;
    memoryBarrier.dstAccessMask = dstAccessMask;
    
    m_memoryBarriers.push_back( memoryBarrier );
}

void VksBarrier::setBarrier(VkCommandBuffer commandBuffer)
{
    uint32_t imageCount = m_imageBarriers.size();
    uint32_t bufferCount = m_bufferBarriers.size();
    uint32_t memoryCount = m_memoryBarriers.size();
    
    const VkImageMemoryBarrier *imageBarrierPtr = imageCount > 0 ? m_imageBarriers.data() : nullptr;
    const VkBufferMemoryBarrier* bufferBarrierPtr = bufferCount > 0 ? m_bufferBarriers.data() : nullptr;
    const VkMemoryBarrier* memoryBarrierPtr = memoryCount > 0 ? m_memoryBarriers.data() : nullptr;
    
    vkCmdPipelineBarrier(commandBuffer, m_srcStage, m_dstStage, 0, memoryCount, memoryBarrierPtr, bufferCount, bufferBarrierPtr, imageCount, imageBarrierPtr);
}
