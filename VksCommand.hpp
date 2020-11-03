#ifndef H_VKSCOMMAND
#define H_VKSCOMMAND

#include "VkEngine.hpp"
#include "VksBuffer.hpp"
#include "VksShaderProgram.hpp"
#include "VksFramebuffer.hpp"
#include "VksGraphicPipeline.hpp"
#include <vector>

class VksCommand : protected VkEngine
{
public:
    static std::shared_ptr<VksCommand> createCommandPool(uint32_t queueFamilyIndex);

    VkCommandBuffer beginOnceSubmitBuffer();
    void endOnceSubmitBuffer( VkCommandBuffer commandbuffer );
    std::vector<VkCommandBuffer> createPrimaryBuffer(int size);
    
    VkCommandBuffer createPrimaryBuffer();

    ~VksCommand();

    VkCommandPool getCommandPool() const;
    
    void beginRenderPass( VkCommandBuffer commandBuffer, const std::shared_ptr<VksFramebuffer> dstFramebuffer );
    void endRenderPass( VkCommandBuffer commandBuffer );
    
    void bindVertexBuffer( VkCommandBuffer commandBuffer, const std::shared_ptr<VksBuffer> vertexBuffer );
    void bindIndexBuffer( VkCommandBuffer commandBuffer, const std::shared_ptr<VksBuffer> indexBuffer, VkIndexType indexValType = VK_INDEX_TYPE_UINT16 );
    void bindUniformSet( VkCommandBuffer commandBuffer, const std::shared_ptr<VksShaderProgram>& shaderPtr, VkDescriptorSet descriptorSet);
    
    void drawIndexed( VkCommandBuffer commandBuffer, uint32_t indexCount );
    void draw( VkCommandBuffer commandBuffer, uint32_t vertexCount );
protected:

private:
    VksCommand( VkCommandPool commandPool );

    VkCommandPool m_commandPool;
};

#endif
