//
//  VksGraphicPipeline.cpp
//  Vulkan
//
//  Created by larry-kof on 2020/7/12.
//  Copyright © 2020 larry. All rights reserved.
//

#include "VksGraphicPipeline.hpp"
#include <iostream>

VksGraphicPipeline::VksGraphicPipeline()
    :VkEngine(), m_colorBlend(), m_depthStencil(), m_graphicPipeline(VK_NULL_HANDLE)
{
    memset(&m_graphicPipelineInfo, 0, sizeof(m_graphicPipelineInfo));
    memset(&m_viewportState, 0, sizeof( m_viewportState ));
    m_graphicPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    m_graphicPipelineInfo.pColorBlendState = &(m_colorBlend.m_colorBlend);
    m_graphicPipelineInfo.pDepthStencilState = &(m_depthStencil.m_depthStencil);
    
    m_viewport.x = 0.0f;
    m_viewport.y = 0.0f;
    m_viewport.width = 1.0f;
    m_viewport.height = 1.0f;
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;
    
    m_scissor.offset = {0,0};
    m_scissor.extent = { 1, 1 };
    
    m_viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_viewportState.viewportCount = 1;
    m_viewportState.pViewports = &m_viewport;
    m_viewportState.scissorCount = 1;
    m_viewportState.pScissors = &m_scissor;
}

VksGraphicPipeline::~VksGraphicPipeline()
{
    if( m_graphicPipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline(m_logicDevice, m_graphicPipeline, nullptr);
    }
}

void VksGraphicPipeline::__addShaderComponent(VksShaderProgram *shader)
{
    auto& shaders = shader->getShaderStageCreateInfo();
    m_graphicPipelineInfo.stageCount = static_cast<uint32_t>( shaders.size() );
    m_graphicPipelineInfo.pStages = shaders.data();
    
//    VkPipelineLayoutCreateInfo pipelineInfo = {};
//    pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    pipelineInfo.setLayoutCount = 1;
//    auto descSetLayout = shader->getDescriptorSetLayout();
//    pipelineInfo.pSetLayouts = &descSetLayout;
//    pipelineInfo.pushConstantRangeCount = 0;
//
//    VK_CHECK( vkCreatePipelineLayout(m_logicDevice, &pipelineInfo, nullptr, &m_pipelineLayout) )
    
    m_graphicPipelineInfo.layout = shader->getPipelineLayout();
}

void VksGraphicPipeline::__addAttributeComponent(VksAttribute *attribute)
{
    auto& inputAssemble = attribute->getInputAssembly();
    auto& vertexStateInfo = attribute->getVertexInputState();
    
    m_graphicPipelineInfo.pInputAssemblyState = &inputAssemble;
    m_graphicPipelineInfo.pVertexInputState = &vertexStateInfo;
}

void VksGraphicPipeline::__addColorBlendComponent(VksColorBlend *colorBlend)
{
    m_colorBlend = (*colorBlend);
    m_graphicPipelineInfo.pColorBlendState = &(m_colorBlend.m_colorBlend);
}

void VksGraphicPipeline::__addDepthStencilComponent(VksDepthStencil *depthStencil)
{
    m_depthStencil = (*depthStencil);
    m_graphicPipelineInfo.pDepthStencilState = &(m_depthStencil.m_depthStencil);
}

void VksGraphicPipeline::__addRenderPass(VksRenderPass *renderPass)
{
    m_graphicPipelineInfo.renderPass = renderPass->getVkRenderPass();
    m_graphicPipelineInfo.subpass = 0;
}

void VksGraphicPipeline::createGraphicPipeline()
{
    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    m_graphicPipelineInfo.pMultisampleState = &multisampling;
    m_graphicPipelineInfo.pRasterizationState = &rasterizationState;
    
    m_graphicPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    m_graphicPipelineInfo.basePipelineIndex = 0;
    
    m_graphicPipelineInfo.pViewportState = &m_viewportState;
    std::vector<VkDynamicState> dynamicStates;
    dynamicStates.push_back( VK_DYNAMIC_STATE_VIEWPORT );
    dynamicStates.push_back( VK_DYNAMIC_STATE_SCISSOR );
    dynamicStates.push_back( VK_DYNAMIC_STATE_LINE_WIDTH );
    dynamicStates.push_back( VK_DYNAMIC_STATE_BLEND_CONSTANTS );
    dynamicStates.push_back( VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK );
    dynamicStates.push_back( VK_DYNAMIC_STATE_STENCIL_WRITE_MASK );
    dynamicStates.push_back( VK_DYNAMIC_STATE_STENCIL_REFERENCE );
    dynamicStates.push_back( VK_DYNAMIC_STATE_DEPTH_BOUNDS );

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>( dynamicStates.size() );
    dynamicState.pDynamicStates = dynamicStates.data();
    
    m_graphicPipelineInfo.pDynamicState = &dynamicState;
    
    VK_CHECK( vkCreateGraphicsPipelines(m_logicDevice, VK_NULL_HANDLE, 1, &m_graphicPipelineInfo, nullptr, &m_graphicPipeline) )
    
}

void VksGraphicPipeline::setSubpassIndex(uint32_t subpassIndex)
{
    m_graphicPipelineInfo.subpass = subpassIndex;
}

void VksGraphicPipeline::setViewport(const VkViewport &viewport, const VkRect2D& scissor )
{
    m_viewport = viewport;
    m_scissor = scissor;
}
