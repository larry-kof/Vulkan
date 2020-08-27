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

class VksTexture : protected VkEngine
{
    
public:
    static std::shared_ptr<VksTexture> createEmptyTexture( uint32_t width, uint32_t height, VkFormat format,
                                               VkImageLayout imageLayout, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                          VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT);
    
    static std::shared_ptr<VksTexture> createFromFile( const char* filePath, VkImageUsageFlags usageFlags, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_GENERAL );
    
    void transferImageLayout( VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessFlag,
                             VkAccessFlags dstAccessFlag, VkPipelineStageFlags srcStageFlag, VkPipelineStageFlags dstStageFlag );
    
    const VkImage& getImage()
    {
        return m_texture;
    }
    
    const VkImageView& getImageView()
    {
        return m_textureView;
    }
    
    const VkSampler& getSampler()
    {
        return m_textureSampler;
    }
    
    const VkDescriptorImageInfo& getDesscriptor()
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
    
private:
    void __createImage( uint32_t width, uint32_t height, VkFormat format, VkImageLayout imageLayout, VkImageUsageFlags usage, VkMemoryPropertyFlags properties );
    
    void __createImageView();
    void __createSampler();
    void __transferImageLayout( VkImageLayout oldLayout, VkImageLayout newLayout );

};

#endif /* VksTexture_hpp */