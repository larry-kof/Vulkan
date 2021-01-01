#include "VkEngine.hpp"
#include <iostream>
#include <set>
#include "VksCommand.hpp"

static bool enableValidationLayers = true;
VkInstance VkEngine::m_instance = VK_NULL_HANDLE;
VkPhysicalDevice VkEngine::m_physicalDevice = VK_NULL_HANDLE;
VkDevice VkEngine::m_logicDevice = VK_NULL_HANDLE;
VkSurfaceKHR VkEngine::m_surface = VK_NULL_HANDLE;
GLFWwindow* VkEngine::m_window = nullptr;
std::atomic_uint VkEngine::m_subCount = 0;
VkQueue VkEngine::m_graphicsQueue = VK_NULL_HANDLE;
VkQueue VkEngine::m_presentQueue = VK_NULL_HANDLE;
QueueFamilyIndices VkEngine::m_familyIndices;
std::shared_ptr<VksCommand> VkEngine::m_graphicCommand = nullptr;
VkDebugUtilsMessengerEXT VkEngine::m_debugMessenger = VK_NULL_HANDLE;

const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VkEngine::VkEngine()
{
    m_subCount.fetch_add(1);
    if( m_instance == VK_NULL_HANDLE )
    {
        __initWindow();
        __createInstance();
        __createSurface();
        __pickPhysicalDevice();
        __createLogicDevice();
    }
}

VkEngine::~VkEngine()
{
    m_subCount.fetch_sub(1);
    if( m_subCount.load() == 1 )
    {
        m_graphicCommand.reset();
    }
    else if( m_subCount.load() == 0 )
    {
        if( m_debugMessenger )
            DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        vkDestroyDevice( m_logicDevice, nullptr );
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance( m_instance, nullptr );
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
}

void VkEngine::__initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
}

void VkEngine::__createInstance()
{
    VkInstanceCreateInfo insInfo = {};
    insInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    uint32_t glfwExtensionCount = 0;
    const char**  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    std::vector<const char*> debugExtenstions( extensions.begin(), extensions.end() );
    if( enableValidationLayers )
    {
        debugExtenstions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
        debugExtenstions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
    }

    insInfo.enabledExtensionCount = static_cast<uint32_t>(debugExtenstions.size());
    insInfo.ppEnabledExtensionNames = debugExtenstions.data();

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.pApplicationName = "VkEngin App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    
    insInfo.pApplicationInfo = &appInfo;

    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> avaiableLayers( layerCount );
    vkEnumerateInstanceLayerProperties(&layerCount, avaiableLayers.data());

    bool found = false;
    for(auto it = avaiableLayers.begin(); it!=avaiableLayers.end(); it++)
    {
        printf("%s\n", it->layerName);
        if( strcmp(it->layerName, validationLayers[0]) == 0)
        {
            found = true;
            break;
        }
    }

    if( found )
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        insInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        insInfo.ppEnabledLayerNames = validationLayers.data();
        __populateDebugMessengerCreateInfo(debugCreateInfo);
        insInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }
    else
    {
        insInfo.enabledLayerCount = 0;
        insInfo.ppEnabledLayerNames = nullptr;
        insInfo.pNext = nullptr;
    }

    VK_CHECK( vkCreateInstance(&insInfo, nullptr, &m_instance) );

    if (!enableValidationLayers) return;
        
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    __populateDebugMessengerCreateInfo(createInfo);
    
    if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void VkEngine::__createSurface()
{
    VK_CHECK( glfwCreateWindowSurface( m_instance, m_window, nullptr, &m_surface) )
}

void VkEngine::__pickPhysicalDevice()
{
    uint physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices( m_instance, &physicalDeviceCount, nullptr );
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

    vkEnumeratePhysicalDevices( m_instance, &physicalDeviceCount, physicalDevices.data() );

    for(auto device : physicalDevices)
    {
        if( __isDeviceSuitable(device) )
        {
            m_physicalDevice = device;
            break;
        }
    }

    if( m_physicalDevice == VK_NULL_HANDLE )
    {
        throw std::runtime_error("");
    }
}

bool VkEngine::__isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = _findQueueFamily(device);
    bool extensionSupport = _checkDeviceExtensionSupport(device);

    return indices.isComplete() && extensionSupport;
}

bool VkEngine::_checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    
    for( auto& extension : availableExtensions )
    {
        if( strncmp(extension.extensionName, deviceExtensions[0], strlen(extension.extensionName)) )
        {
            return true;
        }
    }
    
    return false;
}

QueueFamilyIndices VkEngine::_findQueueFamily(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for(i =0 ; i<queueFamilyCount; i++)
    {
        auto queueFamily = queueFamilies[i];
        if( queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT
            && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT )
        {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupported = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupported);
        if( queueFamily.queueCount > 0 && presentSupported )
        {
            indices.presentFamily = i;
        }
        if(indices.isComplete())
        {
            break;
        }
    }
    return indices;
}

void VkEngine::__createLogicDevice()
{
    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    deviceInfo.enabledExtensionCount = static_cast<uint32_t>( deviceExtensions.size() );
    deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

    deviceInfo.enabledLayerCount = static_cast<uint32_t>( validationLayers.size() );
    deviceInfo.ppEnabledLayerNames = validationLayers.data();

    VkPhysicalDeviceFeatures features = {};
    deviceInfo.pEnabledFeatures = &features;

    m_familyIndices = _findQueueFamily(m_physicalDevice);

    std::set<int> uniqueQueueFamilies = {
        m_familyIndices.graphicsFamily.value(),
        m_familyIndices.presentFamily.value()
    };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for( uint32_t queueFamily : uniqueQueueFamilies )
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>( queueCreateInfos.size() );

    VK_CHECK( vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_logicDevice) )

    vkGetDeviceQueue(m_logicDevice, m_familyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_logicDevice, m_familyIndices.presentFamily.value(), 0, &m_presentQueue);
    
    __createGraphicCommand(m_familyIndices);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    
    return VK_FALSE;
}

void VkEngine::__populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void VkEngine::__createGraphicCommand( const QueueFamilyIndices& indices )
{
    m_graphicCommand = VksCommand::createCommandPool(indices.graphicsFamily.value());
}

std::optional<uint32_t> VkEngine::__findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties ) {
            return i;
        }
    }
    
    return std::nullopt;
}

VksCommand* VkEngine::getGraphicVksCommand()
{
    return m_graphicCommand.get();
}
