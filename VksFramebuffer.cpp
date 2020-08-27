//
//  VksFramebuffer.cpp
//  Vulkan
//
//  Created by larry-kof on 2020/8/14.
//  Copyright © 2020 larry. All rights reserved.
//

#include "VksFramebuffer.hpp"
#include <array>

VksFramebuffer::VksFramebuffer(const VkImageView imageView, uint32_t width, uint32_t height, const std::shared_ptr<VksRenderPass>& renderPass)
    :VkEngine(), m_framebuffer(VK_NULL_HANDLE)
{
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &imageView;
    framebufferInfo.renderPass = renderPass->getVkRenderPass();
    framebufferInfo.layers = 1;
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    
    m_width = width;
    m_height = height;
    
    VK_CHECK( vkCreateFramebuffer(m_logicDevice, &framebufferInfo, nullptr, &m_framebuffer) )
}

VksFramebuffer::VksFramebuffer( const std::shared_ptr<VksTexture>& colorTexture,
                               const std::shared_ptr<VksRenderPass>& renderPass )
    :VkEngine(), m_framebuffer(VK_NULL_HANDLE), m_colorTexture( colorTexture )
{
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.attachmentCount = 1;
    const VkImageView& colorView = colorTexture->getImageView();
    framebufferInfo.pAttachments = &colorView;
    framebufferInfo.renderPass = renderPass->getVkRenderPass();
    framebufferInfo.layers = 1;
    framebufferInfo.width = colorTexture->getWidth();
    framebufferInfo.height = colorTexture->getHeight();
    m_width = framebufferInfo.width;
    m_height = framebufferInfo.height;
    
    VK_CHECK( vkCreateFramebuffer(m_logicDevice, &framebufferInfo, nullptr, &m_framebuffer) )
}

VksFramebuffer::VksFramebuffer( const std::shared_ptr<VksTexture>& colorTexture, const std::shared_ptr<VksTexture>& depthStencilTexture, const std::shared_ptr<VksRenderPass>& renderPass)
    :VkEngine(), m_framebuffer(VK_NULL_HANDLE), m_colorTexture(colorTexture)
    ,m_depthStencilTexture( depthStencilTexture )
{
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.attachmentCount = 2;
    const VkImageView& colorView = colorTexture->getImageView();
    const VkImageView& depthView = depthStencilTexture->getImageView();
    
    std::array<VkImageView, 2> attachments = {
        colorView,
        depthView
    };

    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.renderPass = renderPass->getVkRenderPass();
    framebufferInfo.layers = 1;
    framebufferInfo.width = colorTexture->getWidth();
    framebufferInfo.height = colorTexture->getHeight();
    m_width = framebufferInfo.width;
    m_height = framebufferInfo.height;

    VK_CHECK( vkCreateFramebuffer(m_logicDevice, &framebufferInfo, nullptr, &m_framebuffer) )

}

VksFramebuffer::~VksFramebuffer()
{
    if( m_framebuffer != VK_NULL_HANDLE )
    {
        vkDestroyFramebuffer(m_logicDevice, m_framebuffer, nullptr);
    }
}
