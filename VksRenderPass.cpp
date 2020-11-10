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
:VkEngine(), m_renderPass( VK_NULL_HANDLE ), m_subpassDesc({} )
{
    m_subpassDesc.flags = 0;
    m_subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
}

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

std::shared_ptr<VksRenderPass> VksRenderPass::createColorDepthRenderPass(VkFormat colorFormat, VkImageLayout colorImageLayout, VkFormat depthFormat, VkImageLayout depthImageLayout)
{
    std::shared_ptr<VksRenderPass> renderPass( new VksRenderPass );
    
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = colorFormat;
    colorAttachment.flags = 0;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = colorImageLayout;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    
    
    VkAttachmentReference colorRef;
    colorRef.attachment = 0;
    colorRef.layout = colorImageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL :colorImageLayout ;
    
    VkAttachmentDescription depthAttatchment = {};
    depthAttatchment.format = depthFormat;
    depthAttatchment.flags = 0;
    depthAttatchment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttatchment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttatchment.finalLayout = depthImageLayout;
    depthAttatchment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttatchment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttatchment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttatchment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    
    VkAttachmentReference depthRef;
    depthRef.attachment = 1;
    depthRef.layout = depthImageLayout;
    
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;
    
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
    
    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttatchment };
    
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.pDependencies = dependencies.data();
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    
    VkRenderPass vkRenderPass = VK_NULL_HANDLE;
    vkCreateRenderPass(m_logicDevice, &renderPassInfo, nullptr, &vkRenderPass );
    
    renderPass->m_renderPass = vkRenderPass;
    return renderPass;
}


void VksRenderPass::addColorAttachment(VkFormat attachFormat, VkImageLayout finalImagelayout)
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
    
    VkAttachmentReference ref = {};
    ref.layout = finalImagelayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : finalImagelayout;
    ref.attachment = m_attachDescs.size();
    
    m_attachDescs.push_back( attachment );
    m_colorRefs.push_back( ref );
    
    m_subpassDesc.colorAttachmentCount = m_colorRefs.size();
    m_subpassDesc.pColorAttachments = m_colorRefs.data();
}

void VksRenderPass::addDepthAttachment(VkFormat attachFormat, VkImageLayout imageLayout)
{
    VkAttachmentDescription attachment = {};
    attachment.format = attachFormat;
    attachment.flags = 0;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = imageLayout;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    
    VkAttachmentReference ref = {};
    ref.attachment = m_attachDescs.size();
    ref.layout = imageLayout;
    
    m_depthRef = ref;
    m_attachDescs.push_back( attachment );
    
    m_subpassDesc.pDepthStencilAttachment = &m_depthRef;
}

void VksRenderPass::addSubpassDependency( const std::vector<VkSubpassDependency>& dependencies )
{
    m_dependencies = dependencies;
}

void VksRenderPass::createRenderPass()
{
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.attachmentCount = m_attachDescs.size();
    renderPassInfo.dependencyCount = m_dependencies.size();
    renderPassInfo.pAttachments = m_attachDescs.data();
    renderPassInfo.pDependencies = m_dependencies.data();
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &m_subpassDesc;
    
    VkRenderPass vkRenderPass = VK_NULL_HANDLE;
    vkCreateRenderPass(m_logicDevice, &renderPassInfo, nullptr, &vkRenderPass );
    
    m_renderPass = vkRenderPass;
}

VkRenderPass VksRenderPass::getVkRenderPass()
{
    return m_renderPass;
}
