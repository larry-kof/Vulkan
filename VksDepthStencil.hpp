//
//  VksDepthStencil.hpp
//  Vulkan
//
//  Created by larry-kof on 2020/7/12.
//  Copyright © 2020 larry. All rights reserved.
//

#ifndef VksDepthStencil_hpp
#define VksDepthStencil_hpp

#include "VkEngine.hpp"
class VksGraphicPipeline;
class VksDepthStencil
{
public:
    VksDepthStencil();
    VksDepthStencil(const VksDepthStencil& depthStencil);
    VksDepthStencil& operator=(const VksDepthStencil& depthStencil);
    
    void enableDepthTest( bool enable = true );
    void depthCompareFunc( VkCompareOp depthOp );
    
    // stencil func
protected:
    VkPipelineDepthStencilStateCreateInfo m_depthStencil;
    
    friend class VksGraphicPipeline;
};

#endif /* VksDepthStencil_hpp */
