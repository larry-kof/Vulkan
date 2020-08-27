//
//  VksFramebuffer.hpp
//  Vulkan
//
//  Created by larry-kof on 2020/8/14.
//  Copyright © 2020 larry. All rights reserved.
//

#ifndef VksFramebuffer_hpp
#define VksFramebuffer_hpp

#include <stdio.h>
#include "VkEngine.hpp"
#include "VksTexture.hpp"
#include "VksRenderPass.hpp"

class VksFramebuffer : protected VkEngine
{
public:
    VksFramebuffer( const VkImageView imageView, uint32_t width, uint32_t height, const std::shared_ptr<VksRenderPass>& renderPass );
    VksFramebuffer( const std::shared_ptr<VksTexture>& colorTexture, const std::shared_ptr<VksRenderPass>& renderPass);
    VksFramebuffer( const std::shared_ptr<VksTexture>& colorTexture, const std::shared_ptr<VksTexture>& depthStencilTexture, const std::shared_ptr<VksRenderPass>& renderPass);
    
    ~VksFramebuffer();
    
    VkFramebuffer getVkFramebuffer()
    {
        return m_framebuffer;
    }
    
    VkExtent2D getFramebufferSize()
    {
        VkExtent2D size = { m_width, m_height };
        return size;
    }
    
private:
    VkFramebuffer m_framebuffer;
    
    std::shared_ptr<VksTexture> m_colorTexture;
    std::shared_ptr<VksTexture> m_depthStencilTexture;
    uint32_t m_width;
    uint32_t m_height;
};

#endif /* VksFramebuffer_hpp */
