//
//  VksDepthStencil.cpp
//  Vulkan
//
//  Created by larry-kof on 2020/7/12.
//  Copyright © 2020 larry. All rights reserved.
//

#include "VksDepthStencil.hpp"

VksDepthStencil::VksDepthStencil()
{
    memset( &m_depthStencil, 0, sizeof(m_depthStencil) );
    m_depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_depthStencil.depthTestEnable = VK_FALSE;
    m_depthStencil.stencilTestEnable = VK_FALSE;
    m_depthStencil.depthWriteEnable = VK_FALSE;
    
    m_depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    m_depthStencil.back.failOp = VK_STENCIL_OP_KEEP;
    m_depthStencil.back.passOp = VK_STENCIL_OP_KEEP;
    m_depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;
    m_depthStencil.back.compareMask = 0;
    m_depthStencil.back.reference = 0;
    m_depthStencil.back.depthFailOp = VK_STENCIL_OP_KEEP;
    m_depthStencil.back.writeMask = 0;
    m_depthStencil.front = m_depthStencil.back;
}

VksDepthStencil::VksDepthStencil(const VksDepthStencil& depthStencil)
{
    this->m_depthStencil = depthStencil.m_depthStencil;
}

VksDepthStencil& VksDepthStencil::operator=(const VksDepthStencil &depthStencil)
{
    this->m_depthStencil = depthStencil.m_depthStencil;
    return *this;
}

void VksDepthStencil::enableDepthTest( bool enable )
{
    if( enable )
    {
        m_depthStencil.depthTestEnable = VK_TRUE;
        m_depthStencil.depthWriteEnable = VK_TRUE;
    }
    else
    {
        m_depthStencil.depthTestEnable = VK_FALSE;
        m_depthStencil.depthWriteEnable = VK_FALSE;
    }
}

void VksDepthStencil::depthCompareFunc(VkCompareOp depthOp)
{
    m_depthStencil.depthCompareOp = depthOp;
}
