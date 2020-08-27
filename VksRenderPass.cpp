//
//  VksRenderPass.cpp
//  Vulkan
//
//  Created by larry-kof on 2020/7/13.
//  Copyright © 2020 larry. All rights reserved.
//

#include "VksRenderPass.hpp"
#include <array>

VksRenderPass::VksRenderPass()
:VkEngine(), m_renderPass( VK_NULL_HANDLE )
{}

VksRenderPass::~VksRenderPass()
{
    if( m_renderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass(m_logicDevice, m_renderPass, nullptr);
    }
}

std::shared_ptr<VksRenderPass> VksRenderPass::createSimpleColorAttachmentRenderPass( VkFormat attachFormat )
{
    std::shared_ptr<VksRenderPass> renderPass( new VksRenderPass );
    
    VkAttachmentDescription attachment = {};
    attachment.format = attachFormat;
    attachment.flags = 0;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    
    VkAttachmentReference reference = {};
    reference.attachment = 0;
    reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pColorAttachments = &reference;
    
    std::array<VkSubpassDependency, 2> dependencies;
    
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
    
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pAttachments = &attachment;
    renderPassInfo.pDependencies = dependencies.data();
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    
    VkRenderPass vkRenderPass = VK_NULL_HANDLE;
    vkCreateRenderPass(m_logicDevice, &renderPassInfo, nullptr, &vkRenderPass );
    
    renderPass->m_renderPass = vkRenderPass;
    return renderPass;
}

void VksRenderPass::setAttachmentDescriptors(const std::vector<VkAttachmentDescription> &attachDescs)
{
    m_attechDescs.resize( attachDescs.size() );
    memcpy(m_attechDescs.data(), attachDescs.data(), sizeof(VkAttachmentDescription) * attachDescs.size() );
}

void VksRenderPass::addAllClearOpAttachmentDescriptor(VkFormat attachFormat, VkImageLayout finalImagelayout)
{
    VkAttachmentDescription attachment = {};
    attachment.format = attachFormat;
    attachment.flags = 0;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = finalImagelayout;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    
    m_attechDescs.push_back( attachment );
}

VkRenderPass VksRenderPass::getVkRenderPass()
{
    return m_renderPass;
}