//
//  VksBarrier.h
//  Vulkan
//
//  Created by CL10097-TMP on 2020/11/19.
//  Copyright © 2020 larry. All rights reserved.
//

#ifndef VksBarrier_h
#define VksBarrier_h

#include "VkEngine.hpp"
#include <type_traits>

class VksTexture;
class VksBuffer;
class VksBarrier : protected VkEngine
{
public:
    static std::shared_ptr<VksBarrier> createImageBarrier( const std::shared_ptr<VksTexture>& srcImage,
                                                          VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                          VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage );
    
    static std::shared_ptr<VksBarrier> createBufferBarrier( const std::shared_ptr<VksBuffer>& srcBuffer,
                                                           VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                           VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage );
    
    static std::shared_ptr<VksBarrier> createMemoryBarrier( VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                          VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage );
    
    void addImageBarrier( const std::shared_ptr<VksTexture>& srcImage,
                         VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask );
    
    void addBufferBarrier( const std::shared_ptr<VksBuffer>& srcBuffer,
                          VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask );
    void addMemoryBarrier( VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask  );
    
    void setBarrier( VkCommandBuffer commandBuffer );
private:
    VksBarrier();
    
    void __addImageBarrier( const std::shared_ptr<VksTexture>& srcImage,
                            VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
    
    void __addBufferBarrier( const std::shared_ptr<VksBuffer>& srcBuffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask );
    
    void __addMemoryBarrier( VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask );
    
    std::vector< VkImageMemoryBarrier > m_imageBarriers;
    std::vector< VkBufferMemoryBarrier> m_bufferBarriers;
    std::vector< VkMemoryBarrier > m_memoryBarriers;
    VkPipelineStageFlags m_srcStage;
    VkPipelineStageFlags m_dstStage;
};

#endif /* VksBarrier_h */
