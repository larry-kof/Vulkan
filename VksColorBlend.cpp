//
//  VksColorBlend.cpp
//  Vulkan
//
//  Created by larry-kof on 2020/7/12.
//  Copyright © 2020 larry. All rights reserved.
//

#include "VksColorBlend.hpp"


VksColorBlend::VksColorBlend()
{
    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    
    VkPipelineColorBlendAttachmentState attach = {};
    attach.blendEnable = VK_FALSE;
    attach.colorBlendOp = VK_BLEND_OP_ADD;
    attach.alphaBlendOp = VK_BLEND_OP_ADD;
    attach.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT;
    
    m_colorBlendAttach = attach;
    colorBlendState.pAttachments = &m_colorBlendAttach;
    colorBlendState.attachmentCount = 1;
    colorBlendState.blendConstants[0] = 1.0f;
    colorBlendState.blendConstants[1] = 1.0f;
    colorBlendState.blendConstants[2] = 1.0f;
    colorBlendState.blendConstants[3] = 1.0f;
    
    m_colorBlend = colorBlendState;
}

VksColorBlend::VksColorBlend(const VksColorBlend& colorBlend)
{
    this->m_colorBlend = colorBlend.m_colorBlend;
    this->m_colorBlendAttach = colorBlend.m_colorBlendAttach;
}

VksColorBlend& VksColorBlend::operator=(const VksColorBlend &colorBlend)
{
    this->m_colorBlend = colorBlend.m_colorBlend;
    this->m_colorBlendAttach = colorBlend.m_colorBlendAttach;
    return *this;
}

void VksColorBlend::setBlendOp( VkBlendFactor src, VkBlendFactor dst )
{
    m_colorBlendAttach.blendEnable = VK_TRUE;
    m_colorBlendAttach.srcColorBlendFactor = src;
    m_colorBlendAttach.dstColorBlendFactor = dst;
    m_colorBlendAttach.srcAlphaBlendFactor = src;
    m_colorBlendAttach.dstAlphaBlendFactor = dst;
    
    m_colorBlend.pAttachments = &m_colorBlendAttach;
}

void VksColorBlend::setBlendOp(VkBlendFactor colorSrc, VkBlendFactor colorDst, VkBlendFactor alphaSrc, VkBlendFactor alphaDst)
{
    m_colorBlendAttach.blendEnable = VK_TRUE;
    m_colorBlendAttach.srcColorBlendFactor = colorSrc;
    m_colorBlendAttach.dstColorBlendFactor = colorDst;
    m_colorBlendAttach.srcAlphaBlendFactor = alphaSrc;
    m_colorBlendAttach.dstAlphaBlendFactor = alphaDst;
    
    m_colorBlend.pAttachments = &m_colorBlendAttach;
}

