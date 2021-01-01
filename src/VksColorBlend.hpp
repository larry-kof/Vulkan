//
//  VksColorBlend.hpp
//  Vulkan
//
//  Created by larry-kof on 2020/7/12.
//  Copyright © 2020 larry. All rights reserved.
//

#ifndef VksColorBlend_hpp
#define VksColorBlend_hpp

#include "VkEngine.hpp"

class VksGraphicPipeline;
class VksColorBlend
{
public:
    VksColorBlend();
    
    VksColorBlend(const VksColorBlend& colorBlend);
    VksColorBlend& operator=(const VksColorBlend& colorBlend);
    
    void setBlendOp( VkBlendFactor src, VkBlendFactor dst );
    void setBlendOp( VkBlendFactor colorSrc, VkBlendFactor colorDst, VkBlendFactor alphaSrc, VkBlendFactor alphaDst );
protected:
    VkPipelineColorBlendStateCreateInfo m_colorBlend;
    VkPipelineColorBlendAttachmentState m_colorBlendAttach;
    
    friend class VksGraphicPipeline;
};

#endif /* VksColorBlend_hpp */
