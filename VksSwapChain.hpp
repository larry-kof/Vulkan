#ifndef H_VKSSWAPCHAIN
#define H_VKSSWAPCHAIN

#include "VkEngine.hpp"
#include "VksFramebuffer.hpp"

class VksSwapChain : protected VkEngine {
public:
    VksSwapChain();
    virtual ~VksSwapChain();
    int getSwapChainCount();
    
    std::shared_ptr<VksRenderPass> getSwapChainRenderPass();
    VkExtent2D getRenderAreaSize();
    
    std::shared_ptr<VksFramebuffer> getSwapChainFrameBuffer( int index );
    
    void drawFrames( const std::vector<VkCommandBuffer>& commandBuffers );

private:
    VkSwapchainKHR m_swapchain;
    VkSurfaceFormatKHR m_format;
    VkExtent2D m_extent2D;
    VkPresentModeKHR m_presentMode;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
//    VkRenderPass m_renderPass;
    std::shared_ptr<VksRenderPass> m_renderPass;
//    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    std::vector<std::shared_ptr<VksFramebuffer>> m_swapChainFramebuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphore;
    std::vector<VkSemaphore> m_renderFinishedSemaphore;
    std::vector<VkFence> m_fence;
    std::vector<VkFence> m_imageFence;
    uint32_t m_currentFrame = 0;

    void __chooseFormat();
    void __chooseExtent2D();
    void __createSwapChain();
    void __createImageViews();
    void __createRenderPass();
    void __createFbs();
    void __createSemaphores();
    void __drawFrames( const std::vector<VkCommandBuffer>& commandBuffers );
};

#endif
