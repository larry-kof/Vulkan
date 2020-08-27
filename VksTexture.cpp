//
//  VksTexture.cpp
//  Vulkan
//
//  Created by larry-kof on 2020/8/14.
//  Copyright © 2020 larry. All rights reserved.
//

#include "VksTexture.hpp"
#include "VksCommand.hpp"
#include "VksBuffer.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

VksTexture::VksTexture()
{
}

std::shared_ptr<VksTexture> VksTexture::createEmptyTexture(uint32_t width, uint32_t height, VkFormat format, VkImageLayout imageLayout, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropties,
                                                           VkImageAspectFlags aspectFlag)
{
    std::shared_ptr<VksTexture> texture( new VksTexture() );
    texture->__createImage(width, height, format, VK_IMAGE_LAYOUT_UNDEFINED, usageFlags, memoryPropties);
    texture->m_width = width;
    texture->m_height = height;
    texture->m_format = format;
    texture->m_aspectFlag = aspectFlag;
    
    texture->__transferImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, imageLayout);
    
    texture->__createImageView();
    texture->__createSampler();
    
    texture->m_descriptor.imageLayout = imageLayout;
    texture->m_descriptor.imageView = texture->m_textureView;
    texture->m_descriptor.sampler = texture->m_textureSampler;
    
    return texture;
}

std::shared_ptr<VksTexture> VksTexture::createFromFile(const char *filePath, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout )
{
    std::shared_ptr<VksTexture> texture( new VksTexture() );
    
    int texWidth = 0, texHeight = 0, texChannel = 0;
    stbi_uc* pixel = stbi_load(filePath, &texWidth, &texHeight, &texChannel, STBI_rgb_alpha);
    
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    
    auto stagingBuffer = VksBuffer::createBuffer(imageSize,
                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
    void* imageData;
    stagingBuffer->mapMemory(0, imageSize, &imageData);
    memcpy( imageData, pixel, imageSize );
    stagingBuffer->unMapMemory();
    
    stbi_image_free(pixel);
    
    texture->__createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_TRANSFER_DST_BIT | imageUsageFlags , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    texture->m_width = texWidth;
    texture->m_height = texHeight;
    texture->m_format = VK_FORMAT_R8G8B8A8_UNORM;
    
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    texture->m_aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;

    region.imageExtent = {0,0,0};
    region.imageExtent = { (uint32_t)texWidth, (uint32_t)texHeight, 1 };
    
    VkCommandBuffer commandBuffer = m_graphicCommand->beginOnceSubmitBuffer();
    
    texture->__transferImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer->getVkBuffer(), texture->m_texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    if( imageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
        texture->__transferImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageLayout);
    m_graphicCommand->endOnceSubmitBuffer(commandBuffer);
    
    texture->__createImageView();
    texture->__createSampler();
    
    texture->m_descriptor.imageLayout = imageLayout;
    texture->m_descriptor.imageView = texture->m_textureView;
    texture->m_descriptor.sampler = texture->m_textureSampler;
    
    return texture;
}

void VksTexture::__createImageView()
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.format = m_format;
    viewInfo.image = m_texture;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    viewInfo.subresourceRange.aspectMask = m_aspectFlag;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    
    VK_CHECK( vkCreateImageView(m_logicDevice, &viewInfo, nullptr, &m_textureView) )
}

void VksTexture::__createSampler()
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.maxAnisotropy = 1;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    
    VK_CHECK( vkCreateSampler(m_logicDevice, &samplerInfo, nullptr, &m_textureSampler) )
}

void VksTexture::__createImage(uint32_t width, uint32_t height, VkFormat format, VkImageLayout imageLayout, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.format = format;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.flags = 0;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.initialLayout = imageLayout;
    imageInfo.arrayLayers = 1;
    imageInfo.mipLevels = 1;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.usage = usage;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    
    VK_CHECK( vkCreateImage(m_logicDevice, &imageInfo, nullptr, &m_texture) )
    
    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(m_logicDevice, m_texture, &memReq);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    
    auto memoryTypeIndex = __findMemoryType(memReq.memoryTypeBits, properties);
    if( memoryTypeIndex == std::nullopt )
    {
        throw std::runtime_error(" Can not find related memory property flags ");
    }
    
    allocInfo.memoryTypeIndex = memoryTypeIndex.value();
    
    vkAllocateMemory(m_logicDevice, &allocInfo, nullptr, &m_textureMemory);
    
    vkBindImageMemory(m_logicDevice, m_texture, m_textureMemory, 0);
}

void VksTexture::transferImageLayout( VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessFlag,
                         VkAccessFlags dstAccessFlag, VkPipelineStageFlags srcStageFlag, VkPipelineStageFlags dstStageFlag )
{
    VkImageMemoryBarrier imageBarrier = {};
    imageBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    imageBarrier.oldLayout = oldLayout;
    imageBarrier.newLayout = newLayout;
    imageBarrier.image = m_texture;
    imageBarrier.srcAccessMask = srcAccessFlag;
    imageBarrier.dstAccessMask = dstAccessFlag;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.subresourceRange.aspectMask = m_aspectFlag;
    imageBarrier.subresourceRange.baseArrayLayer = 0;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.subresourceRange.layerCount = 1;
    imageBarrier.subresourceRange.levelCount = 1;
    
    VkCommandBuffer commandBuffer = m_graphicCommand->beginOnceSubmitBuffer();
    vkCmdPipelineBarrier(commandBuffer, srcStageFlag, dstStageFlag, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier );

    m_graphicCommand->endOnceSubmitBuffer(commandBuffer);
}

void VksTexture::__transferImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier imageBarrier = {};
    imageBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    imageBarrier.oldLayout = oldLayout;
    imageBarrier.newLayout = newLayout;
    imageBarrier.image = m_texture;
    imageBarrier.srcAccessMask = 0;
    imageBarrier.dstAccessMask = 0;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange.baseArrayLayer = 0;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.subresourceRange.layerCount = 1;
    imageBarrier.subresourceRange.levelCount = 1;
    
    VkPipelineStageFlags srcStageFlag = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStageFlag = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    
    VkCommandBuffer commandBuffer = m_graphicCommand->beginOnceSubmitBuffer();
    
    if( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED )
    {
        imageBarrier.srcAccessMask = 0;
        srcStageFlag = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    else if( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
    {
        imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStageFlag = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL )
    {
        imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        srcStageFlag = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if( oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL )
    {
        imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        srcStageFlag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    
    if( newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
    {
        imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstStageFlag = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if( newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL )
    {
        imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        dstStageFlag = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if( newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR )
    {
    }
    else if( newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL )
    {
        imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dstStageFlag = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    else if( newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
    {
        imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dstStageFlag = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    
    
    vkCmdPipelineBarrier(commandBuffer, srcStageFlag, dstStageFlag, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier );
    
    m_graphicCommand->endOnceSubmitBuffer(commandBuffer);
}
