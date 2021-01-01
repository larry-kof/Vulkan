//
//  VksFramebuffer.hpp
//  Vulkan
//
//  Created by larry-kof on 2020/8/14.
//  Copyright © 2020 larry. All rights reserved.
//

#ifndef VksFramebuffer_hpp
#define VksFramebuffer_hpp

#include <stdio.h>
#include "VkEngine.hpp"

class VksBarrier;
class VksTexture;
class VksRenderPass;
class VksGraphicPipeline;
class VksBuffer;
class VksFramebuffer : protected VkEngine, public std::enable_shared_from_this<VksFramebuffer>
{
public:
    VksFramebuffer( const std::shared_ptr<VksTexture>& colorTexture, const std::shared_ptr<VksRenderPass>& renderPass);
    VksFramebuffer( const std::shared_ptr<VksTexture>& colorTexture, const std::shared_ptr<VksTexture>& depthStencilTexture, const std::shared_ptr<VksRenderPass>& renderPass);
    
    void useGraphicPipeline( const std::shared_ptr<VksGraphicPipeline>& graphicPipeline );
    
    ~VksFramebuffer();
    
    VkFramebuffer getVkFramebuffer();

    VkExtent2D getFramebufferSize();

    const VkCommandBuffer getVkCommandBuffer();

    VkRenderPass getVkRenderPass();

    const std::shared_ptr<VksTexture> getColorTexture();
    const std::shared_ptr<VksTexture> getDepthStencilTexture();

    void bind( std::shared_ptr<VksBarrier> barrier = nullptr );
    void unBind();
    
    void bindUniformSets( int setsIndex );
    void bindVertexBuffer( const std::shared_ptr<VksBuffer>& vertexBuffer );
    void bindIndexBuffer( const std::shared_ptr<VksBuffer>& indexBuffer );
    void draw( int vertexCount );
    void drawIndexed( int indexCount );
    
    void submitRender( const std::vector<VkSemaphore>& waitSemaphores, std::vector<VkPipelineStageFlags>& waitStages,
                      std::vector<VkSemaphore>& signalSemaphores );
    
private:
    VkFramebuffer m_framebuffer;
    
    std::shared_ptr<VksTexture> m_colorTexture;
    std::shared_ptr<VksTexture> m_depthStencilTexture;
    std::shared_ptr<VksRenderPass> m_rendePass;
    std::shared_ptr<VksGraphicPipeline> m_graphicPipeline;
    VkCommandBuffer m_commandBuffer;
    VkSemaphore m_renderComplete;
    VkFence m_fence;
    uint32_t m_width;
    uint32_t m_height;
};

#endif /* VksFramebuffer_hpp */
