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
    
    void addColorAttachment(VkFormat attachFormat, VkImageLayout finalImagelayout);
    void addDepthAttachment(VkFormat attachFormat, VkImageLayout imageLayout);
    void addSubpassDependency( const std::vector<VkSubpassDependency>& dependencies );
    void createRenderPass();

    VkRenderPass getVkRenderPass();
private:
    VkRenderPass m_renderPass;
    std::vector<VkAttachmentDescription> m_attachDescs;
    std::vector<VkAttachmentReference> m_colorRefs;
    VkAttachmentReference m_depthRef;
 
    VkSubpassDescription m_subpassDesc;
    std::vector<VkSubpassDependency> m_dependencies;
};

#endif /* VksRenderPass_hpp */
