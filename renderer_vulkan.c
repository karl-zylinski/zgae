#include <vulkan/vulkan.h>
#include "renderer.h"
#include "linux_xcb_window.h"
#include "memory.h"
#include "debug.h"
#include "math.h"

#define VERIFY_RES() check(res == VK_SUCCESS, "VULKAN ERROR")

typedef struct {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkFormat surface_format;
    VkPhysicalDevice gpu;
    VkPhysicalDeviceProperties gpu_properties;
    VkPhysicalDeviceMemoryProperties gpu_memory_properties;
    VkDevice device;
    VkSwapchainKHR swapchain;
    uint32_t graphics_queue_family_idx;
    VkQueue graphics_queue;
    uint32_t present_queue_family_idx;
    VkQueue present_queue;
} vk_state_t;

struct renderer_state {
    vk_state_t vk_state;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_message_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* user_data)
{
    if (severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        info("Vulkan info: %s", data->pMessage);
    else
        error("Vulkan error: %s", data->pMessage);
    return VK_FALSE;
}

static vec2u_t get_surface_size(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_capabilities);
    VERIFY_RES();
    check(surface_capabilities.currentExtent.width != -1, "Couldn't get surface size");
    vec2u_t size = {surface_capabilities.currentExtent.width, surface_capabilities.currentExtent.height};
    return size;
}

static VkPresentModeKHR choose_swapchain_present_mode(VkPhysicalDevice gpu, VkSurfaceKHR surface) {
    VkResult res;
    uint32_t present_modes_count;
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_modes_count, NULL);
    VERIFY_RES();
    VkPresentModeKHR* present_modes = mema(present_modes_count * sizeof(VkPresentModeKHR));
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_modes_count, present_modes);
    VERIFY_RES();

    for (uint32_t i = 0; i < present_modes_count; ++i)
    {
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            memf(present_modes);
            return present_modes[i];
        }
    }

    memf(present_modes);
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkCompositeAlphaFlagBitsKHR choose_swapchain_composite_alpha(VkCompositeAlphaFlagsKHR alpha_flags)
{
    if (alpha_flags & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
        return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    if (alpha_flags & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
        return VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;

    if (alpha_flags & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
        return VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;

    return VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
}

static VkSwapchainKHR create_swapchain(
    VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface, VkFormat format,
    uint32_t graphics_queue_family_idx, uint32_t present_queue_family_idx, VkSwapchainKHR old_swapchain)
{
    vec2u_t size = get_surface_size(gpu, surface);
    info("Creating swapchain with size %dx%d", size.x, size.y);
    VkResult res;

    VkSurfaceCapabilitiesKHR surface_capabilities;
    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_capabilities);
    VERIFY_RES();

    VkSurfaceTransformFlagBitsKHR pre_transform = 
        surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
        ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
        : surface_capabilities.currentTransform;

    VkSwapchainCreateInfoKHR scci = {};
    scci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    scci.surface = surface;
    scci.minImageCount = surface_capabilities.minImageCount;
    scci.imageFormat = format;
    scci.imageExtent.width = size.x;
    scci.imageExtent.height = size.y;
    scci.preTransform = pre_transform;
    scci.compositeAlpha = choose_swapchain_composite_alpha(surface_capabilities.supportedCompositeAlpha);
    scci.imageArrayLayers = 1;
    scci.presentMode = choose_swapchain_present_mode(gpu, surface);
    scci.oldSwapchain = old_swapchain;
    scci.clipped = 1;
    scci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    scci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    scci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    uint32_t queue_family_indicies[] = {graphics_queue_family_idx, present_queue_family_idx};

    if (queue_family_indicies[0] != queue_family_indicies[1])
    {
        scci.pQueueFamilyIndices = queue_family_indicies;
        scci.queueFamilyIndexCount = 2;
        scci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    }

    VkSwapchainKHR swapchain;
    res = vkCreateSwapchainKHR(device, &scci, NULL, &swapchain);
    VERIFY_RES();
    return swapchain;
}

static VkFormat choose_surface_format(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
    VkResult res;
    uint32_t num_supported_surface_formats;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &num_supported_surface_formats, NULL);
    VERIFY_RES();
    VkSurfaceFormatKHR* supported_surface_formats = mema(num_supported_surface_formats * sizeof(VkSurfaceFormatKHR));
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &num_supported_surface_formats, supported_surface_formats);
    VERIFY_RES();

    // This means we can choose ourselves.
    if (num_supported_surface_formats == 1 && supported_surface_formats[0].format == VK_FORMAT_UNDEFINED)
    {
        memf(supported_surface_formats);
        return VK_FORMAT_R8G8B8A8_UNORM;
    }

    check(num_supported_surface_formats > 0, "No supported surface formats found");

    for (uint32_t i = 0; i < num_supported_surface_formats; ++i)
    {
        if (supported_surface_formats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
        {
            memf(supported_surface_formats);
            return VK_FORMAT_R8G8B8A8_UNORM;
        }
    }

    VkFormat format = supported_surface_formats[0].format;
    memf(supported_surface_formats);
    return format;
}

static VkPhysicalDevice choose_gpu(VkPhysicalDevice* gpus, uint32_t num_gpus)
{
    check(num_gpus > 0, "Trying to select among 0 GPUs");

    if (num_gpus == 1)
        return gpus[0];

    for (uint32_t i = 0; i < num_gpus; ++i)
    {
        VkPhysicalDeviceProperties gpu_properties;
        vkGetPhysicalDeviceProperties(gpus[i], &gpu_properties);

        if (gpu_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            return gpus[i];
    }

    return gpus[0];
}

typedef VkResult (*vkCreateDebugUtilsMessengerEXT_t)(VkInstance, const VkDebugUtilsMessengerCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*);
typedef void (*vkDestroyDebugUtilsMessengerEXT_t)(VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*);

renderer_state_t* renderer_init(window_type_t window_type, void* window_data)
{
    info("Creating Vulkan renderer");

    check(window_type == WINDOW_TYPE_XCB, "passed window_type_e must be WINDOW_TYPE_XCB");
    renderer_state_t* rs = memaz(sizeof(renderer_state_t));
    vk_state_t* vks = &rs->vk_state;
    VkResult res;

    info("Creating Vulkan instance and debug callback");

    VkApplicationInfo ai = {};
    ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ai.pApplicationName = "VulkanTest";
    ai.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    ai.apiVersion = VK_API_VERSION_1_0;

    VkDebugUtilsMessengerCreateInfoEXT debug_ext_ci = {};
    debug_ext_ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_ext_ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_ext_ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_ext_ci.pfnUserCallback = vulkan_debug_message_callback;

    const char* const validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
    const char* const extensions[] = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XCB_SURFACE_EXTENSION_NAME};
    
    VkInstanceCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &ai;
    ici.ppEnabledExtensionNames = extensions;
    ici.enabledExtensionCount = sizeof(extensions)/sizeof(char*);
    ici.ppEnabledLayerNames = validation_layers;
    ici.enabledLayerCount = sizeof(validation_layers)/sizeof(char*);
    ici.pNext = &debug_ext_ci;
    res = vkCreateInstance(&ici, NULL, &vks->instance);
    VERIFY_RES();
    VkInstance instance = vks->instance;

    vkCreateDebugUtilsMessengerEXT_t vkCreateDebugUtilsMessengerEXT = (vkCreateDebugUtilsMessengerEXT_t)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    res = vkCreateDebugUtilsMessengerEXT(instance, &debug_ext_ci, NULL, &vks->debug_messenger);
    VERIFY_RES();

    info("Creating XCB Vulkan surface");
    linux_xcb_window_t* win = window_data;
    VkXcbSurfaceCreateInfoKHR xcbci = {};
    xcbci.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    xcbci.connection = win->connection;
    xcbci.window = win->handle;
    res = vkCreateXcbSurfaceKHR(instance, &xcbci, NULL, &vks->surface);
    VERIFY_RES();
    VkSurfaceKHR surface = vks->surface;

    info("Selecting GPU and fetching properties");
    uint32_t num_available_gpus = 0;
    res = vkEnumeratePhysicalDevices(instance, &num_available_gpus, NULL);
    check(res == VK_SUCCESS || res == VK_INCOMPLETE, "Failed enumerating GPUs");
    check(num_available_gpus > 0, "No GPUS found");
    VkPhysicalDevice* available_gpus = mema(sizeof(VkPhysicalDevice) * num_available_gpus);
    res = vkEnumeratePhysicalDevices(instance, &num_available_gpus, available_gpus);
    VERIFY_RES();
    VkPhysicalDevice gpu = choose_gpu(available_gpus, num_available_gpus);
    vks->gpu = gpu;
    vkGetPhysicalDeviceProperties(vks->gpu, &vks->gpu_properties);
    vkGetPhysicalDeviceMemoryProperties(vks->gpu, &vks->gpu_memory_properties);
    info("Selected GPU %s", vks->gpu_properties.deviceName);

    info("Finding GPU graphics and present queue families");
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, NULL);
    VkQueueFamilyProperties* queue_family_props = mema(sizeof(VkQueueFamilyProperties) * queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, queue_family_props);
    
    VkBool32* queues_with_present_support = mema(queue_family_count * sizeof(VkBool32));
    for (uint32_t i = 0; i < queue_family_count; ++i)
    {
        res = vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &queues_with_present_support[i]);
        VERIFY_RES();
    }

    vks->graphics_queue_family_idx = -1;
    vks->present_queue_family_idx = -1;

    // We first try to find a queue family with both graphics and present capabilities,
    // if it fails we try to look for two separate queue families
    for (uint32_t i = 0; i < queue_family_count; ++i)
    {
        if (queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            vks->graphics_queue_family_idx = i;

            if (queues_with_present_support[i] == VK_TRUE)
            {
                vks->present_queue_family_idx = i;
                info("Found combined queue family");
                break;
            }
        }
    }
    check(vks->graphics_queue_family_idx != -1, "Couldn't find graphics queue family");
    if (vks->present_queue_family_idx == -1)
    {
        for (uint32_t i = 0; i < queue_family_count; ++i)
        {
            if (queues_with_present_support[i] == VK_TRUE)
            {
                vks->present_queue_family_idx = i;
                info("Found two separate queue families");
                break;
            }
        }
    }
    check(vks->present_queue_family_idx != -1, "Couldn't find present queue family");
    memf(queues_with_present_support);

    info("Creating Vulkan logical device");
    VkDeviceQueueCreateInfo dqci = {};
    dqci.queueFamilyIndex = vks->graphics_queue_family_idx;
    dqci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    dqci.queueCount = 1;
    float queue_priorities[] = {0.0};
    dqci.pQueuePriorities = queue_priorities;

    const char* const device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo dci = {};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &dqci;
    dci.ppEnabledExtensionNames = device_extensions;
    dci.enabledExtensionCount = sizeof(device_extensions)/sizeof(char*);

    res = vkCreateDevice(gpu, &dci, NULL, &vks->device);
    VERIFY_RES();
    VkDevice device = vks->device;

    vks->surface_format = choose_surface_format(gpu, surface);
    info("Chose surface VkFormat: %d", vks->surface_format);

    vks->swapchain = create_swapchain(gpu, device, surface, vks->surface_format, vks->graphics_queue_family_idx, vks->present_queue_family_idx, NULL);

    info("Creating graphics and present queues");
    vkGetDeviceQueue(device, vks->graphics_queue_family_idx, 0, &vks->graphics_queue);
    if (vks->graphics_queue_family_idx == vks->present_queue_family_idx)
        vks->present_queue = vks->graphics_queue;
    else
        vkGetDeviceQueue(device, vks->present_queue_family_idx, 0, &vks->present_queue);

    return rs;
}

void renderer_shutdown(renderer_state_t* rs)
{
    info("Destroying Vulkan renderer");
    vk_state_t* vks = &rs->vk_state;
    VkDevice device = vks->device;
    vkDestroySwapchainKHR(device, vks->swapchain, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroyDebugUtilsMessengerEXT_t vkDestroyDebugUtilsMessengerEXT = (vkDestroyDebugUtilsMessengerEXT_t)vkGetInstanceProcAddr(vks->instance, "vkDestroyDebugUtilsMessengerEXT");
    vkDestroyDebugUtilsMessengerEXT(vks->instance, vks->debug_messenger, NULL);
    vkDestroyInstance(vks->instance, NULL);
    memf(rs);
}