//
//  VksRenderPass.hpp
//  Vulkan
//
//  Created by larry-kof on 2020/7/13.
//  Copyright © 2020 larry. All rights reserved.
//

#ifndef VksRenderPass_hpp
#define VksRenderPass_hpp

#include "VkEngine.hpp"

class VksRenderPass : private VkEngine
{
public:
    VksRenderPass();
    ~VksRenderPass();
    static std::shared_ptr<VksRenderPass> createSimpleColorAttachmentRenderPass( VkFormat colorFormat );
    static std::shared_ptr<VksRenderPass> createColorDepthRenderPass( VkFormat colorFormat, VkImageLayout colorImageLayout, VkFormat depthFormat, VkImageLayout depthImageLayout );
    
    void setAttachmentDescriptors(const std::vector<VkAttachmentDescription> &attachDescs);
    void addAllClearOpAttachmentDescriptor(VkFormat attachFormat, VkImageLayout finalImagelayout);
    
    VkRenderPass getVkRenderPass();
private:
    VkRenderPass m_renderPass;
    std::vector<VkAttachmentDescription> m_attechDescs;
};

#endif /* VksRenderPass_hpp */
