//
//  VksFramebuffer.cpp
//  Vulkan
//
//  Created by larry-kof on 2020/8/14.
//  Copyright © 2020 larry. All rights reserved.
//

#include "VksCommand.hpp"
#include "VksTexture.hpp"
#include "VksRenderPass.hpp"
#include "VksGraphicPipeline.hpp"
#include "VksBarrier.hpp"
#include "VksBuffer.hpp"
#include "VksFramebuffer.hpp"
#include <array>
#include <iostream>

VksFramebuffer::VksFramebuffer( const std::shared_ptr<VksTexture>& colorTexture,
                               const std::shared_ptr<VksRenderPass>& renderPass )
    :VkEngine(), m_framebuffer(VK_NULL_HANDLE), m_colorTexture( colorTexture )
    ,m_commandBuffer( VK_NULL_HANDLE )
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
    
    m_rendePass = renderPass;
    
    VK_CHECK( vkCreateFramebuffer(m_logicDevice, &framebufferInfo, nullptr, &m_framebuffer) )
    m_commandBuffer = m_graphicCommand->createPrimaryBuffer();
    
    VkSemaphoreCreateInfo semaphoreCreate = {};
    semaphoreCreate.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VK_CHECK( vkCreateSemaphore(m_logicDevice, &semaphoreCreate, nullptr, &m_renderComplete) )
    
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    VK_CHECK( vkCreateFence(m_logicDevice, &fenceInfo, nullptr, &m_fence) )
}

VksFramebuffer::VksFramebuffer( const std::shared_ptr<VksTexture>& colorTexture, const std::shared_ptr<VksTexture>& depthStencilTexture, const std::shared_ptr<VksRenderPass>& renderPass)
    :VkEngine(), m_framebuffer(VK_NULL_HANDLE), m_colorTexture(colorTexture)
    ,m_depthStencilTexture( depthStencilTexture )
    ,m_commandBuffer( VK_NULL_HANDLE )
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
    
    m_rendePass = renderPass;

    VK_CHECK( vkCreateFramebuffer(m_logicDevice, &framebufferInfo, nullptr, &m_framebuffer) )
    m_commandBuffer = m_graphicCommand->createPrimaryBuffer();
    
    VkSemaphoreCreateInfo semaphoreCreate = {};
    semaphoreCreate.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VK_CHECK( vkCreateSemaphore(m_logicDevice, &semaphoreCreate, nullptr, &m_renderComplete) )
    
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    VK_CHECK( vkCreateFence(m_logicDevice, &fenceInfo, nullptr, &m_fence) )
}

VkFramebuffer VksFramebuffer::getVkFramebuffer()
{
    return m_framebuffer;
}

VkExtent2D VksFramebuffer::getFramebufferSize()
{
    VkExtent2D size = { m_width, m_height };
    return size;
}

const VkCommandBuffer VksFramebuffer::getVkCommandBuffer()
{
    return m_commandBuffer;
}

VkRenderPass VksFramebuffer::getVkRenderPass()
{
    return m_rendePass->getVkRenderPass();
}

void VksFramebuffer::useGraphicPipeline(const std::shared_ptr<VksGraphicPipeline> &graphicPipeline)
{
    m_graphicPipeline = graphicPipeline;
    m_graphicPipeline->addComponent<VksRenderPass>( m_rendePass.get() );
    m_graphicPipeline->__createGraphicPipeline();
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline->getVkGraphicPipele());
}

void VksFramebuffer::bind( std::shared_ptr<VksBarrier> barrier )
{
    m_graphicCommand->beginRenderPass(m_commandBuffer, shared_from_this(), barrier);
}

void VksFramebuffer::unBind()
{
    m_graphicCommand->endRenderPass(m_commandBuffer);
}

void VksFramebuffer::bindUniformSets(int setsIndex)
{
    auto shader = m_graphicPipeline->m_weakShader.lock();
    if( !shader ) return;
    m_graphicCommand->bindUniformSet(m_commandBuffer, shader, shader->getDescriptorSet( setsIndex ));
}

void VksFramebuffer::bindIndexBuffer(const std::shared_ptr<VksBuffer> &indexBuffer)
{
    m_graphicCommand->bindIndexBuffer(m_commandBuffer, indexBuffer);
}

void VksFramebuffer::bindVertexBuffer(const std::shared_ptr<VksBuffer> &vertexBuffer)
{
    m_graphicCommand->bindVertexBuffer(m_commandBuffer, vertexBuffer);
}

void VksFramebuffer::draw(int vertexCount)
{
    m_graphicCommand->draw(m_commandBuffer, vertexCount);
}

void VksFramebuffer::drawIndexed(int indexCount)
{
    m_graphicCommand->drawIndexed(m_commandBuffer, indexCount);
}

void VksFramebuffer::submitRender(std::vector<VkSemaphore> &waitSemaphores, std::vector<VkPipelineStageFlags> &waitStages, std::vector<VkSemaphore> &signalSemaphores)
{
    vkWaitForFences(m_logicDevice, 1, &m_fence, VK_TRUE, UINT_MAX);
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    
    VkPipelineStageFlags *waitDstStageMasks = new VkPipelineStageFlags[ waitStages.size() + 2 ];
    waitDstStageMasks[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    waitDstStageMasks[1] = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    
    for( int i = 0; i < waitStages.size(); i++ )
    {
        waitDstStageMasks[ 2 + i ] = waitStages.at(i);
    }
    
    submitInfo.pWaitDstStageMask = waitDstStageMasks;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;
    VkSemaphore signalSema[] = { m_renderComplete };
    
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSema;
    
    signalSemaphores.push_back( m_renderComplete );
    vkResetFences(m_logicDevice, 1, &m_fence);
    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_fence);
    delete [] waitDstStageMasks;
}

VksFramebuffer::~VksFramebuffer()
{
    if( m_framebuffer != VK_NULL_HANDLE )
    {
        vkDestroyFramebuffer(m_logicDevice, m_framebuffer, nullptr);
    }
    
    if( m_commandBuffer != VK_NULL_HANDLE )
    {
        vkFreeCommandBuffers(m_logicDevice, m_graphicCommand->getCommandPool(), 1, &m_commandBuffer);
    }
    
    if( m_renderComplete != VK_NULL_HANDLE )
    {
        vkDestroySemaphore(m_logicDevice, m_renderComplete, nullptr);
    }
    
    if( m_fence != VK_NULL_HANDLE )
    {
        vkDestroyFence(m_logicDevice, m_fence, nullptr);
    }
}
