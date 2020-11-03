#ifndef H_VKSSWAPCHAIN
#define H_VKSSWAPCHAIN

#include "VkEngine.hpp"
#include "VksFramebuffer.hpp"

class VksSwapChain : protected VkEngine {
public:
    VksSwapChain();
    virtual ~VksSwapChain();
    int getSwapChainCount();
    
    VkExtent2D getRenderAreaSize();
    
    std::shared_ptr<VksFramebuffer> getSwapChainFrameBuffer( int index );
    
    void drawFrames();

private:
    VkSwapchainKHR m_swapchain;
    VkSurfaceFormatKHR m_format;
    VkExtent2D m_extent2D;
    VkPresentModeKHR m_presentMode;
    std::vector<VkImage> m_swapChainImages;
    std::vector<std::shared_ptr<VksTexture>> m_swapChainColorTextures;
    std::shared_ptr<VksTexture> m_swapChainDepthTexture;
    std::shared_ptr<VksRenderPass> m_renderPass;
    std::vector<std::shared_ptr<VksFramebuffer>> m_swapChainFramebuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphore;
    std::vector<VkSemaphore> m_renderFinishedSemaphore;
    std::vector<VkFence> m_fence;
    std::vector<VkFence> m_imageFence;
    uint32_t m_currentFrame = 0;

    void __chooseFormat();
    void __chooseExtent2D();
    void __createSwapChain();
    void __createColorTextures();
    void __createDepthTextures();
    void __createRenderPass();
    void __createFbs();
    void __createSemaphores();
    void __drawFrames();
};

#endif
