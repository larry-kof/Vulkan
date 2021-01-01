//
//  VksTexture.hpp
//  Vulkan
//
//  Created by larry-kof on 2020/8/14.
//  Copyright © 2020 larry. All rights reserved.
//

#ifndef VksTexture_hpp
#define VksTexture_hpp

#include <stdio.h>
#include "VkEngine.hpp"

class VksBuffer;
class VksTexture : protected VkEngine
{
    
public:
    static constexpr bool value = true;
    
    ~VksTexture();
    
    static std::shared_ptr<VksTexture> createEmptyTexture( uint32_t width, uint32_t height, VkFormat format,
                                               VkImageLayout imageLayout, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                          VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT);
    
    static std::shared_ptr<VksTexture> createFromFile( const char* filePath, VkImageUsageFlags usageFlags, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_GENERAL );
    
    static std::shared_ptr<VksTexture> createFromVkImage( VkImage vkImage, uint32_t width, uint32_t height, VkFormat format, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_GENERAL, VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT );
    
    void transferImageLayout( VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessFlag,
                             VkAccessFlags dstAccessFlag, VkPipelineStageFlags srcStageFlag, VkPipelineStageFlags dstStageFlag );
    
    const VkImage& getImage() const
    {
        return m_texture;
    }
    
    const VkImageView& getImageView() const
    {
        return m_textureView;
    }
    
    const VkSampler& getSampler() const
    {
        return m_textureSampler;
    }
    
    const VkDescriptorImageInfo& getDesscriptor() const
    {
        return m_descriptor;
    }
    
    uint32_t getWidth()
    {
        return m_width;
    }
    
    uint32_t getHeight()
    {
        return m_height;
    }
    
    VkFormat getFormat()
    {
        return m_format;
    }
    
    VkImageSubresourceRange getSubresourceRange();

    std::optional<uint32_t> getFamilyIndex()
    {
        return m_familyIndices.graphicsFamily;
    }
    
    void updateTexture( const char* data, VkDeviceSize dataSize, VkOffset2D imageOffset, VkExtent2D imageExtent );
    void updateTexture( const char* filePath );
    
    void updateTexture( const std::shared_ptr<VksBuffer>& buffer, VkOffset2D imageOffset, VkExtent2D imageExtent );
private:
    VksTexture();

    VkImage m_texture;
    VkImageView m_textureView;
    VkDeviceMemory m_textureMemory;
    VkSampler m_textureSampler;
    VkDescriptorImageInfo m_descriptor;
    VkFormat m_format;
    uint32_t m_width;
    uint32_t m_height;
    VkImageAspectFlags m_aspectFlag;
    bool m_ownTexture;

private:
    void __createImage( uint32_t width, uint32_t height, VkFormat format, VkImageLayout imageLayout, VkImageUsageFlags usage, VkMemoryPropertyFlags properties );
    
    void __createImageView();
    void __createSampler();
    void __transferImageLayout( VkImageLayout oldLayout, VkImageLayout newLayout );

};

#endif /* VksTexture_hpp */
