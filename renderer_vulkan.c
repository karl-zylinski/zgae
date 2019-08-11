#include "renderer.h"
#include <assert.h>
#include <vulkan/vulkan.h>
#include "linux_xcb_window.h"
#include "memory.h"
#include "debug.h"

typedef struct {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice gpu;
    VkPhysicalDeviceProperties gpu_properties;
    VkPhysicalDeviceMemoryProperties gpu_memory_properties;
    VkDevice device;
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

static VkPhysicalDevice select_best_gpu(VkPhysicalDevice* gpus, uint32_t num_gpus)
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

renderer_state_t* renderer_init(window_type_e window_type, void* window_data)
{
    info("Creating Vulkan renderer");

    check(window_type == WINDOW_TYPE_XCB, "passed window_type_e must be WINDOW_TYPE_XCB");
    renderer_state_t* rs = memaz(sizeof(renderer_state_t));
    vk_state_t* vks = &rs->vk_state;
    VkResult res;
    #define VERIFY_RES() check(res == VK_SUCCESS, "VULKAN ERROR")

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

    info("Selecting and enumerating GPU");
    uint32_t num_available_gpus = 0;
    res = vkEnumeratePhysicalDevices(instance, &num_available_gpus, NULL);
    check(res == VK_SUCCESS || res == VK_INCOMPLETE, "Failed enumerating GPUs");
    check(num_available_gpus > 0, "No GPUS found");
    VkPhysicalDevice* available_gpus = mema(sizeof(VkPhysicalDevice) * num_available_gpus);
    res = vkEnumeratePhysicalDevices(instance, &num_available_gpus, available_gpus);
    VERIFY_RES();
    VkPhysicalDevice gpu = select_best_gpu(available_gpus, num_available_gpus);
    vks->gpu = gpu;
    vkGetPhysicalDeviceProperties(vks->gpu, &vks->gpu_properties);
    vkGetPhysicalDeviceMemoryProperties(vks->gpu, &vks->gpu_memory_properties);
    info("Selected GPU %s", vks->gpu_properties.deviceName);

    info("Finding GPU graphics and present queues");
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, NULL);
    VkQueueFamilyProperties* queue_family_props = mema(sizeof(VkQueueFamilyProperties) * queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, queue_family_props);
    
    VkBool32* queues_with_present_support = mema(queue_family_count * sizeof(VkBool32));
    for (uint32_t i = 0; i < queue_family_count; ++i)
    {
        res = vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, vks->surface, &queues_with_present_support[i]);
        VERIFY_RES();
    }

    uint32_t graphics_queue_family_idx = -1;
    uint32_t present_queue_family_idx = -1;

    // We first try to find a queue family with both graphics and present capabilities,
    // if it fails we try to look for two separate queue families
    for (uint32_t i = 0; i < queue_family_count; ++i)
    {
        if (queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphics_queue_family_idx = i;

            if (queues_with_present_support[i] == VK_TRUE)
            {
                present_queue_family_idx = i;
                break;
            }
        }
    }
    check(graphics_queue_family_idx != -1, "Couldn't find graphics queue family");
    if (present_queue_family_idx == -1)
    {
        for (uint32_t i = 0; i < queue_family_count; ++i)
        {
            if (queues_with_present_support[i] == VK_TRUE)
            {
                present_queue_family_idx = i;
                break;
            }
        }
    }
    check(present_queue_family_idx != -1, "Couldn't find present queue family");
    memf(queues_with_present_support);

    info("Creating Vulkan logical device");
    VkDeviceQueueCreateInfo dqci = {};
    dqci.queueFamilyIndex = graphics_queue_family_idx;
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

    info("SOMETHING")
    uint32_t supported_surface_formats_count;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[0], surface, &supported_surface_formats_count, NULL);
    assert(res == VK_SUCCESS);
    VkSurfaceFormatKHR* supported_surface_formats = malloc(supported_surface_formats_count * sizeof(VkSurfaceFormatKHR));
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[0], surface, &supported_surface_formats_count, supported_surface_formats);
    assert(res == VK_SUCCESS);

    VkFormat format;
    if (supported_surface_formats_count == 1 && supported_surface_formats[0].format == VK_FORMAT_UNDEFINED)
    {
        format = VK_FORMAT_R8G8B8A8_UNORM;
    }
    else
    {
        assert(supported_surface_formats_count >= 1);
        format = supported_surface_formats[0].format;
    }
    free(supported_surface_formats);

    return rs;
}

void renderer_shutdown(renderer_state_t* rs)
{
    info("Destroying Vulkan renderer");
    VkInstance instance = rs->vk_state.instance;
    vk_state_t* vks = &rs->vk_state;
    vkDestroyDebugUtilsMessengerEXT_t vkDestroyDebugUtilsMessengerEXT = (vkDestroyDebugUtilsMessengerEXT_t)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    vkDestroyDebugUtilsMessengerEXT(instance, vks->debug_messenger, NULL);
    vkDestroyInstance(instance, NULL);
    memf(rs);
}