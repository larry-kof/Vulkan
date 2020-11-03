//
//  VksGraphicPipeline.hpp
//  Vulkan
//
//  Created by larry-kof on 2020/7/12.
//  Copyright © 2020 larry. All rights reserved.
//

#ifndef VksGraphicPipeline_hpp
#define VksGraphicPipeline_hpp

#include "VksShaderProgram.hpp"
#include "VksAttribute.hpp"
#include "VksColorBlend.hpp"
#include "VksDepthStencil.hpp"
#include "VkEngine.hpp"
#include "VksRenderPass.hpp"
#include <type_traits>

class VksGraphicPipeline : private VkEngine
{
public:
    VksGraphicPipeline();
    ~VksGraphicPipeline();
    
    template< typename T >
    void addComponent( void* component )
    {
        if( std::is_same<T, VksShaderProgram>::value )
        {
            __addShaderComponent( reinterpret_cast<VksShaderProgram*>(component) );
        }
        else if( std::is_same<T, VksAttribute>::value )
        {
            __addAttributeComponent( reinterpret_cast<VksAttribute*>(component) );
        }
        else if( std::is_same<T, VksColorBlend>::value )
        {
            __addColorBlendComponent( reinterpret_cast<VksColorBlend*>(component) );
        }
        else if( std::is_same<T, VksDepthStencil>::value )
        {
            __addDepthStencilComponent( reinterpret_cast<VksDepthStencil*>(component) );
        }
        else if( std::is_same<T, VksRenderPass>::value )
        {
            __addRenderPass( reinterpret_cast<VksRenderPass*>(component) );
        }
    }
    
    void setSubpassIndex( uint32_t subpassIndex );
    VkPipeline getVkGraphicPipele()
    {
        return m_graphicPipeline;
    }
protected:
    void __addShaderComponent( VksShaderProgram* shader );
    void __addAttributeComponent( VksAttribute* attribute );
    void __addColorBlendComponent( VksColorBlend* colorBlend );
    void __addDepthStencilComponent( VksDepthStencil* depthStencil );
    void __addRenderPass( VksRenderPass* renderPass );
    
    void __createGraphicPipeline();
    friend class VksFramebuffer;
    
    VkGraphicsPipelineCreateInfo m_graphicPipelineInfo;
    VksColorBlend m_colorBlend;
    VksDepthStencil m_depthStencil;
    VkPipeline m_graphicPipeline;
    
    VkPipelineViewportStateCreateInfo m_viewportState;
    VkViewport m_viewport;
    VkRect2D m_scissor;
    
    std::weak_ptr<VksShaderProgram> m_weakShader;
};

#endif /* VksGraphicPipeline_hpp */
