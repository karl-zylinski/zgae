#include <vulkan/vulkan.h>
#include "renderer.h"
#include "linux_xcb_window.h"
#include "memory.h"
#include "debug.h"
#include "math.h"

#define VERIFY_RES() check(res == VK_SUCCESS, "Vulkan error (VkResult is %d)", res)

typedef struct {
    VkImage image;
    VkImageView view;
} swapchain_buffer_t;

typedef struct {
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
} depth_buffer_t;

struct renderer_state {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkFormat surface_format;
    VkPhysicalDevice gpu;
    VkPhysicalDeviceProperties gpu_properties;
    VkPhysicalDeviceMemoryProperties gpu_memory_properties;
    VkDevice device;
    VkSwapchainKHR swapchain;
    vec2u_t swapchain_size;
    swapchain_buffer_t* swapchain_buffers;
    uint32_t swapchain_buffers_count;
    uint32_t graphics_queue_family_idx;
    VkQueue graphics_queue;
    uint32_t present_queue_family_idx;
    VkQueue present_queue;
    VkCommandPool graphics_cmd_pool;
    VkCommandBuffer graphics_cmd_buffer;
    depth_buffer_t depth_buffer;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_message_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* user_data)
{
    if (severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        info("(From Vulkan) %s", data->pMessage);
    else
        error("(From Vulkan) %s", data->pMessage);
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

static void create_swapchain(
    VkSwapchainKHR* out_current_swapchain, swapchain_buffer_t** out_sc_bufs, uint32_t* out_sc_bufs_count, vec2u_t* out_swapchain_size,
    VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface, VkFormat format,
    uint32_t graphics_queue_family_idx, uint32_t present_queue_family_idx, VkSwapchainKHR old_swapchain)
{
    vec2u_t size = get_surface_size(gpu, surface);
    info("Creating swapchain with size %dx%d", size.x, size.y);
    *out_swapchain_size = size;
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
    scci.oldSwapchain = *out_current_swapchain;
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
    *out_current_swapchain = swapchain;

    uint32_t swapchain_image_count;
    res = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, NULL);
    VERIFY_RES();
    check(swapchain_image_count > 0, "Swapchain contains no images");
    VkImage* swapchain_images = mema(swapchain_image_count * sizeof(VkImage));
    res = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images);
    VERIFY_RES();

    info("Creating %d buffers for swapchain", swapchain_image_count);
    swapchain_buffer_t* old_bufs = *out_sc_bufs;
    uint32_t old_bufs_count = *out_sc_bufs_count;

    for (uint32_t i = 0; i < old_bufs_count; i++)
        vkDestroyImageView(device, old_bufs[i].view, NULL);

    swapchain_buffer_t* bufs = memraz(old_bufs, sizeof(swapchain_buffer_t) * swapchain_image_count);

    for (uint32_t i = 0; i < swapchain_image_count; ++i)
    {
        bufs[i].image = swapchain_images[i];

        VkImageViewCreateInfo vci = {};
        vci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vci.image = bufs[i].image;
        vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vci.format = format;
        vci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vci.subresourceRange.levelCount = 1;
        vci.subresourceRange.layerCount = 1;

        res = vkCreateImageView(device, &vci, NULL, &bufs[i].view);
        VERIFY_RES();
    }

    memf(swapchain_images);
    *out_sc_bufs = bufs;
    *out_sc_bufs_count = swapchain_image_count;
}

int memory_type_from_properties(uint32_t req_memory_type, const VkPhysicalDeviceMemoryProperties* memory_properties, VkMemoryPropertyFlags memory_requirement_mask)
{
    for (uint32_t i = 0; i < memory_properties->memoryTypeCount; ++i)
    {
        if ((req_memory_type & (1 << i)) && (memory_properties->memoryTypes[i].propertyFlags & memory_requirement_mask) == memory_requirement_mask)
        {
            return i;
        }
    }

    return -1;
}

void destroy_depth_buffer(VkDevice device, const depth_buffer_t* depth_buffer)
{
    if (!depth_buffer->view && !depth_buffer->image && !depth_buffer->memory)
        return;

    info("Destroying depth buffer");
    vkDestroyImageView(device, depth_buffer->view, NULL);
    vkDestroyImage(device, depth_buffer->image, NULL);
    vkFreeMemory(device, depth_buffer->memory, NULL);
}

static void create_depth_buffer(depth_buffer_t* out_depth_buffer, VkDevice device, VkPhysicalDevice gpu, const VkPhysicalDeviceMemoryProperties* memory_properties, vec2u_t size)
{
    destroy_depth_buffer(device, out_depth_buffer);
    info("Creating depth buffer");
    depth_buffer_t depth_buffer = {};

    VkImageCreateInfo depth_ici = {};
    const VkFormat depth_format = VK_FORMAT_D16_UNORM;
    VkFormatProperties depth_format_props;
    vkGetPhysicalDeviceFormatProperties(gpu, depth_format, &depth_format_props);
    if (depth_format_props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        depth_ici.tiling = VK_IMAGE_TILING_LINEAR;
    else if (depth_format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        depth_ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    else
        error("VK_FORMAT_D16_UNORM unsupported");

    depth_ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depth_ici.imageType = VK_IMAGE_TYPE_2D;
    depth_ici.format = depth_format;
    depth_ici.extent.width = size.x;
    depth_ici.extent.height = size.y;
    depth_ici.extent.depth = 1;
    depth_ici.mipLevels = 1;
    depth_ici.arrayLayers = 1;
    
    #define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

    depth_ici.samples = NUM_SAMPLES;
    depth_ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_ici.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth_ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult res;
    res = vkCreateImage(device, &depth_ici, NULL, &depth_buffer.image);
    VERIFY_RES();

    VkMemoryRequirements depth_mem_reqs;
    vkGetImageMemoryRequirements(device, depth_buffer.image, &depth_mem_reqs);

    VkMemoryAllocateInfo depth_mai = {};
    depth_mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    depth_mai.allocationSize = depth_mem_reqs.size;
    depth_mai.memoryTypeIndex = memory_type_from_properties(depth_mem_reqs.memoryTypeBits, memory_properties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    check(depth_mai.memoryTypeIndex != -1, "Failed to find memory type for depth buffer");
    res = vkAllocateMemory(device, &depth_mai, NULL, &depth_buffer.memory);
    VERIFY_RES();
    res = vkBindImageMemory(device, depth_buffer.image, depth_buffer.memory, 0);
    VERIFY_RES();

    VkImageViewCreateInfo depth_ivci = {};
    depth_ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depth_ivci.image = VK_NULL_HANDLE;
    depth_ivci.format = depth_format;
    depth_ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth_ivci.subresourceRange.levelCount = 1;
    depth_ivci.subresourceRange.layerCount = 1;
    depth_ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depth_ivci.image = depth_buffer.image;
    res = vkCreateImageView(device, &depth_ivci, NULL, &depth_buffer.view);
    VERIFY_RES();
    *out_depth_buffer = depth_buffer;
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
    res = vkCreateInstance(&ici, NULL, &rs->instance);
    VERIFY_RES();
    VkInstance instance = rs->instance;

    vkCreateDebugUtilsMessengerEXT_t vkCreateDebugUtilsMessengerEXT = (vkCreateDebugUtilsMessengerEXT_t)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    res = vkCreateDebugUtilsMessengerEXT(instance, &debug_ext_ci, NULL, &rs->debug_messenger);
    VERIFY_RES();

    info("Creating XCB Vulkan surface");
    linux_xcb_window_t* win = window_data;
    VkXcbSurfaceCreateInfoKHR xcbci = {};
    xcbci.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    xcbci.connection = win->connection;
    xcbci.window = win->handle;
    res = vkCreateXcbSurfaceKHR(instance, &xcbci, NULL, &rs->surface);
    VERIFY_RES();
    VkSurfaceKHR surface = rs->surface;

    info("Selecting GPU and fetching properties");
    uint32_t num_available_gpus = 0;
    res = vkEnumeratePhysicalDevices(instance, &num_available_gpus, NULL);
    check(res == VK_SUCCESS || res == VK_INCOMPLETE, "Failed enumerating GPUs");
    check(num_available_gpus > 0, "No GPUS found");
    VkPhysicalDevice* available_gpus = mema(sizeof(VkPhysicalDevice) * num_available_gpus);
    res = vkEnumeratePhysicalDevices(instance, &num_available_gpus, available_gpus);
    VERIFY_RES();
    VkPhysicalDevice gpu = choose_gpu(available_gpus, num_available_gpus);
    rs->gpu = gpu;
    vkGetPhysicalDeviceProperties(rs->gpu, &rs->gpu_properties);
    vkGetPhysicalDeviceMemoryProperties(rs->gpu, &rs->gpu_memory_properties);
    info("Selected GPU %s", rs->gpu_properties.deviceName);

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

    rs->graphics_queue_family_idx = -1;
    rs->present_queue_family_idx = -1;

    // We first try to find a queue family with both graphics and present capabilities,
    // if it fails we try to look for two separate queue families
    for (uint32_t i = 0; i < queue_family_count; ++i)
    {
        if (queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            rs->graphics_queue_family_idx = i;

            if (queues_with_present_support[i] == VK_TRUE)
            {
                rs->present_queue_family_idx = i;
                info("Found combined queue family");
                break;
            }
        }
    }
    check(rs->graphics_queue_family_idx != -1, "Couldn't find graphics queue family");
    if (rs->present_queue_family_idx == -1)
    {
        for (uint32_t i = 0; i < queue_family_count; ++i)
        {
            if (queues_with_present_support[i] == VK_TRUE)
            {
                rs->present_queue_family_idx = i;
                info("Found two separate queue families");
                break;
            }
        }
    }
    check(rs->present_queue_family_idx != -1, "Couldn't find present queue family");
    memf(queues_with_present_support);

    info("Creating Vulkan logical device");
    VkDeviceQueueCreateInfo dqci = {};
    dqci.queueFamilyIndex = rs->graphics_queue_family_idx;
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

    res = vkCreateDevice(gpu, &dci, NULL, &rs->device);
    VERIFY_RES();
    VkDevice device = rs->device;

    rs->surface_format = choose_surface_format(gpu, surface);
    info("Chose surface VkFormat: %d", rs->surface_format);

    create_swapchain(
        &rs->swapchain, &rs->swapchain_buffers, &rs->swapchain_buffers_count, &rs->swapchain_size,
        gpu, device, surface,
        rs->surface_format, rs->graphics_queue_family_idx,
        rs->present_queue_family_idx, NULL);

    info("Creating graphics and present queues");
    vkGetDeviceQueue(device, rs->graphics_queue_family_idx, 0, &rs->graphics_queue);
    if (rs->graphics_queue_family_idx == rs->present_queue_family_idx)
        rs->present_queue = rs->graphics_queue;
    else
        vkGetDeviceQueue(device, rs->present_queue_family_idx, 0, &rs->present_queue);

    info("Creating graphics queue command pool and command buffer");
    VkCommandPoolCreateInfo cpci = {};
    cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpci.queueFamilyIndex = rs->graphics_queue_family_idx;
    res = vkCreateCommandPool(device, &cpci, NULL, &rs->graphics_cmd_pool);
    VERIFY_RES();

    VkCommandBufferAllocateInfo cbai = {};
    cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbai.commandPool = rs->graphics_cmd_pool;
    cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandBufferCount = 1;
    res = vkAllocateCommandBuffers(device, &cbai, &rs->graphics_cmd_buffer);
    VERIFY_RES();

    create_depth_buffer(&rs->depth_buffer, device, gpu, &rs->gpu_memory_properties, rs->swapchain_size);

    return rs;
}

void renderer_shutdown(renderer_state_t* rs)
{
    info("Destroying Vulkan renderer");
    VkDevice device = rs->device;
    vkFreeCommandBuffers(device, rs->graphics_cmd_pool, 1, &rs->graphics_cmd_buffer);

    destroy_depth_buffer(device, &rs->depth_buffer);
    vkDestroyCommandPool(device, rs->graphics_cmd_pool, NULL);
    for (uint32_t i = 0; i < rs->swapchain_buffers_count; i++)
        vkDestroyImageView(device, rs->swapchain_buffers[i].view, NULL);
    vkDestroySwapchainKHR(device, rs->swapchain, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroyDebugUtilsMessengerEXT_t vkDestroyDebugUtilsMessengerEXT = (vkDestroyDebugUtilsMessengerEXT_t)vkGetInstanceProcAddr(rs->instance, "vkDestroyDebugUtilsMessengerEXT");
    vkDestroyDebugUtilsMessengerEXT(rs->instance, rs->debug_messenger, NULL);
    vkDestroyInstance(rs->instance, NULL);
    memf(rs);
}