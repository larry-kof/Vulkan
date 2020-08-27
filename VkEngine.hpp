#ifndef H_VKENGINE
#define H_VKENGINE

#include <vulkan/vulkan.h>
#include <vector>
#include <stdio.h>
#include <atomic>
#include <exception>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VK_CHECK( result ) \
{ \
    if( (result) != VK_SUCCESS ) { \
        printf( "Error for func = %s at line = %d\n", #result, __LINE__ ); \
        throw std::runtime_error(#result); \
    } \
}

    struct QueueFamilyIndices {
        std::optional<int> graphicsFamily;
        std::optional<int> presentFamily;
        bool isComplete(){
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

class VksCommand;
class VkEngine{
private:
    static VkInstance m_instance;
    static std::atomic_uint m_subCount;
    static VkDebugUtilsMessengerEXT m_debugMessenger;
    
    void __initWindow();
    void __createInstance( );
    void __populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void __createSurface();
    void __pickPhysicalDevice();
    bool __isDeviceSuitable(VkPhysicalDevice device);
    void __createLogicDevice();
    void __createGraphicCommand( const QueueFamilyIndices& indices );

protected:
    static GLFWwindow* m_window;
    static VkSurfaceKHR m_surface;
    static VkPhysicalDevice m_physicalDevice;
    static VkDevice m_logicDevice;

    static VkQueue m_graphicsQueue;
    static VkQueue m_presentQueue;
    
    static std::shared_ptr<VksCommand> m_graphicCommand;

    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    QueueFamilyIndices _findQueueFamily(VkPhysicalDevice device);
    bool _checkDeviceExtensionSupport(VkPhysicalDevice device);
    
    std::optional<uint32_t> __findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

public:
    VkEngine();
    virtual ~VkEngine();
    
    static VksCommand* getGraphicVksCommand();

};
#endif
