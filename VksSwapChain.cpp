#include "VksSwapChain.hpp"
#include <iostream>
#include <array>
#include <chrono>

const int MAX_FLIGHT_IMAGE_COUNT = 2;

VksSwapChain::VksSwapChain()
    : VkEngine()
{
    __createSwapChain();
    __createColorTextures();
    __createDepthTextures();
    __createRenderPass();
    __createFbs();
    __createSemaphores();
}

VksSwapChain::~VksSwapChain()
{
    
    for(int i = 0; i<MAX_FLIGHT_IMAGE_COUNT; i++)
    {
        vkDestroySemaphore(m_logicDevice, m_imageAvailableSemaphore[i], nullptr);
        vkDestroySemaphore(m_logicDevice, m_renderFinishedSemaphore[i], nullptr);
        vkDestroyFence(m_logicDevice, m_fence[i], nullptr);
    }
    m_swapChainColorTextures.clear();
    m_swapChainFramebuffers.clear();
    
    vkDestroySwapchainKHR(m_logicDevice, m_swapchain, nullptr);
}

int VksSwapChain::getSwapChainCount()
{
    return static_cast<int>(m_swapChainFramebuffers.size());
}

VkExtent2D VksSwapChain::getRenderAreaSize()
{
    return m_extent2D;
}

std::shared_ptr<VksFramebuffer> VksSwapChain::getSwapChainFrameBuffer(int index)
{
    return m_swapChainFramebuffers[ index ];
}

void VksSwapChain::__createFbs()
{
    for (int i = 0; i < m_swapChainColorTextures.size(); i++) {
        auto frameBufferPtr = std::make_shared<VksFramebuffer>( m_swapChainColorTextures[i], m_swapChainDepthTexture,  m_renderPass );
        m_swapChainFramebuffers.push_back( frameBufferPtr );
    }
}

void VksSwapChain::__createSwapChain()
{
    __chooseFormat();
    __chooseExtent2D();

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.clipped = true;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.imageArrayLayers = 1;
    createInfo.imageColorSpace = m_format.colorSpace;
    createInfo.imageExtent = m_extent2D;
    createInfo.imageFormat = m_format.format;

    QueueFamilyIndices indices = _findQueueFamily(m_physicalDevice);
    if( indices.graphicsFamily.value() == indices.presentFamily.value() )
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        uint32_t queueFamilayIndices[2] = { 
            static_cast<uint32_t>(indices.graphicsFamily.value()), 
            static_cast<uint32_t>(indices.presentFamily.value()) };
        createInfo.pQueueFamilyIndices = queueFamilayIndices;
    }

    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkSurfaceCapabilitiesKHR cap;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_physicalDevice, m_surface, &cap ); 
    uint32_t imageCount = cap.minImageCount;
    if (cap.maxImageCount > 0 && imageCount > cap.maxImageCount) {
        imageCount = cap.maxImageCount;
    }

    createInfo.minImageCount = imageCount;
    createInfo.presentMode = m_presentMode;
    createInfo.surface = m_surface;
    createInfo.preTransform = cap.currentTransform; 

    VK_CHECK( vkCreateSwapchainKHR(m_logicDevice, &createInfo, nullptr, &m_swapchain) );

    uint32_t imagesCount;
    vkGetSwapchainImagesKHR(m_logicDevice, m_swapchain, &imagesCount, nullptr);
    m_swapChainImages.resize(imagesCount);
    vkGetSwapchainImagesKHR(m_logicDevice, m_swapchain, &imagesCount, m_swapChainImages.data());
}

void VksSwapChain::__createColorTextures()
{
    m_swapChainColorTextures.resize(m_swapChainImages.size());
    for( int i = 0; i < m_swapChainImages.size(); i++ )
    {
        auto colorTexture = VksTexture::createFromVkImage(m_swapChainImages[i], m_extent2D.width, m_extent2D.height, m_format.format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);

        m_swapChainColorTextures[i] = colorTexture;
    }
}

void VksSwapChain::__createDepthTextures()
{
    auto depthTexture = VksTexture::createEmptyTexture(m_extent2D.width, m_extent2D.height, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT );
        
    m_swapChainDepthTexture = depthTexture;
}

void VksSwapChain::__createRenderPass()
{
#if 0
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    VkAttachmentDescription attachmentDesc = {};
    attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachmentDesc.flags = 0;
    attachmentDesc.format = m_format.format;
    attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &attachmentDesc;

    VkSubpassDescription subpassDesc = {};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount = 1;
    VkAttachmentReference colorRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    subpassDesc.pColorAttachments = &colorRef;

    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    std::array<VkSubpassDependency, 2> dependencies;
        
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies.data();

    VK_CHECK( vkCreateRenderPass(m_logicDevice, &renderPassInfo, nullptr, &m_renderPass) )
#endif
//    m_renderPass = VksRenderPass::createSimpleColorAttachmentRenderPass( m_format.format );
    m_renderPass = VksRenderPass::createColorDepthRenderPass(m_format.format,
                                                             VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                             VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void VksSwapChain::__chooseExtent2D()
{
    VkSurfaceCapabilitiesKHR cap;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_physicalDevice, m_surface, &cap );

    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    actualExtent.width = std::max( cap.minImageExtent.width, std::min( actualExtent.width, cap.maxImageExtent.width ) );

    actualExtent.height = std::max( cap.minImageExtent.height, std::min( actualExtent.height, cap.maxImageExtent.height ) );

    m_extent2D = actualExtent;

    uint32_t prsentModeCount ;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &prsentModeCount, nullptr);

    std::vector<VkPresentModeKHR> presentModes( prsentModeCount );
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &prsentModeCount, presentModes.data());

    m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for(auto presentMode : presentModes)
    {
        if( presentMode == VK_PRESENT_MODE_MAILBOX_KHR )
        {
            m_presentMode = presentMode;
            break;
        }
        else if( presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR )
        {
            m_presentMode = presentMode;
            break;
        }
    }

}

void VksSwapChain::__chooseFormat()
{
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
    std::vector< VkSurfaceFormatKHR > formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data());

    if( formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED )
    {
        m_format.format = VK_FORMAT_B8G8R8A8_UNORM;
        m_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    else
    {
        for(const auto& format: formats)
        {
            if( format.format == VK_FORMAT_B8G8R8A8_UNORM 
            && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
            {
                m_format = format;
            }
        }
        m_format = formats[0];
    }
}

void VksSwapChain::__createSemaphores()
{
    m_imageAvailableSemaphore.resize(MAX_FLIGHT_IMAGE_COUNT);
    m_renderFinishedSemaphore.resize(MAX_FLIGHT_IMAGE_COUNT);
    m_fence.resize(MAX_FLIGHT_IMAGE_COUNT);
    m_imageFence.resize(m_swapChainImages.size(), VK_NULL_HANDLE);
    
    for(int i = 0; i < MAX_FLIGHT_IMAGE_COUNT; i++)
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        VK_CHECK( vkCreateSemaphore(m_logicDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore[i] ) )
        VK_CHECK( vkCreateSemaphore(m_logicDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore[i] ) )
        
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_CHECK( vkCreateFence(m_logicDevice, &fenceInfo, nullptr, &m_fence[i]) )
    }
    
}

void VksSwapChain::drawFrames( std::function<void (float timeStamp)> callback)
{
    while( !glfwWindowShouldClose( m_window ) )
    {
        auto start = std::chrono::system_clock::now();
        glfwPollEvents();
        __drawFrames();
        auto end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast< std::chrono::milliseconds>(end - start);
        callback( duration.count() );
    }
    
    vkDeviceWaitIdle(m_logicDevice);
}

void VksSwapChain::drawFrames()
{
    while( !glfwWindowShouldClose( m_window ) )
    {
        glfwPollEvents();
        __drawFrames();
    }
    
    vkDeviceWaitIdle(m_logicDevice);
}

void VksSwapChain::__drawFrames()
{
    uint32_t imageIndex = 0;
    vkWaitForFences(m_logicDevice, 1, &m_fence[ m_currentFrame ], VK_TRUE, UINT_MAX);
    
    vkAcquireNextImageKHR(m_logicDevice, m_swapchain, UINT32_MAX, m_imageAvailableSemaphore[ m_currentFrame ], VK_NULL_HANDLE, &imageIndex);
    if( m_imageFence[imageIndex] != VK_NULL_HANDLE )
    {
        vkWaitForFences(m_logicDevice, 1, &m_imageFence[imageIndex], VK_TRUE, UINT32_MAX);
    }
    m_imageFence[imageIndex] = m_fence[m_currentFrame];
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore[m_currentFrame]};
    VkPipelineStageFlags waitStage[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.commandBufferCount = 1;
    submitInfo.pWaitDstStageMask = waitStage;
    VkCommandBuffer commonBuffer = m_swapChainFramebuffers.at(imageIndex)->getVkCommandBuffer();
    submitInfo.pCommandBuffers = &commonBuffer;
    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore[m_currentFrame]};
    
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    vkResetFences(m_logicDevice, 1, &m_fence[m_currentFrame]);
 
    VK_CHECK( vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_fence[m_currentFrame]) )
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.pImageIndices = &imageIndex;
    VkSwapchainKHR swapChains[] = { m_swapchain };
    presentInfo.pSwapchains = swapChains;
    presentInfo.pResults = nullptr;
    presentInfo.swapchainCount = 1;
    
    VK_CHECK( vkQueuePresentKHR(m_presentQueue, &presentInfo) )
    
    m_currentFrame = (m_currentFrame+1) % MAX_FLIGHT_IMAGE_COUNT;
}
