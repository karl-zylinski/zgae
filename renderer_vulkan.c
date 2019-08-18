#include <vulkan/vulkan.h>
#include "renderer.h"
#include "linux_xcb_window.h"
#include "memory.h"
#include "debug.h"
#include "math.h"
#include "window.h"
#include "handle_pool.h"
#include "shader.h"
#include "pipeline.h"
#include "array.h"
#include <string.h>

#define VERIFY_RES() check(res == VK_SUCCESS, "Vulkan error (VkResult is %d)", res)

typedef struct swapchain_buffer_t
{
    VkImage image;
    VkImageView view;
} swapchain_buffer_t;

typedef struct depth_buffer_t
{
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
} depth_buffer_t;

typedef enum renderer_resource_type_t
{
    RENDERER_RESOURCE_TYPE_INVALID,
    RENDERER_RESOURCE_TYPE_SHADER,
    RENDERER_RESOURCE_TYPE_PIPELINE
} renderer_resource_type_t;

typedef struct shader_t
{
    shader_type_t type;
    VkShaderModule module;
    shader_input_layout_item_t* input_layout;
    uint32_t input_layout_num;
    shader_constant_buffer_t constant_buffer;
} shader_t;

typedef struct pipeline_t
{
    VkBuffer* constant_buffers;
    uint32_t constant_buffers_num;
    VkPipeline vk_handle;
} pipeline_t;

typedef struct renderer_resource_t
{
    renderer_resource_handle_t handle;
    renderer_resource_type_t type;
    union
    {
        shader_t shader;
        pipeline_t pipeline;
    };
} renderer_resource_t;

typedef struct renderer_state_t
{
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
    renderer_resource_t* da_resources;
    handle_pool_t resource_handle_pool;
} renderer_state_t;

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_message_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* user_data)
{
    (void)type;
    (void)user_data;

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
    check(surface_capabilities.currentExtent.width != (uint32_t)-1, "Couldn't get surface size");
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
    uint32_t graphics_queue_family_idx, uint32_t present_queue_family_idx)
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

    swapchain_buffer_t* bufs = memra_zero(old_bufs, sizeof(swapchain_buffer_t) * swapchain_image_count);

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

uint32_t memory_type_from_properties(uint32_t req_memory_type, const VkPhysicalDeviceMemoryProperties* memory_properties, VkMemoryPropertyFlags memory_requirement_mask)
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

    check(depth_mai.memoryTypeIndex != (uint32_t)-1, "Failed to find memory type for depth buffer");
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

typedef VkResult (*fptr_vkCreateDebugUtilsMessengerEXT)(VkInstance, const VkDebugUtilsMessengerCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*);
typedef void (*fptr_vkDestroyDebugUtilsMessengerEXT)(VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*);

renderer_state_t* renderer_create(window_type_t window_type, void* window_data)
{
    info("Creating Vulkan renderer");

    check(window_type == WINDOW_TYPE_XCB, "passed window_type_e must be WINDOW_TYPE_XCB");
    renderer_state_t* rs = mema_zero(sizeof(renderer_state_t));
    handle_pool_init(&rs->resource_handle_pool);
    #define stringify(m) (#m)
    handle_pool_set_type(&rs->resource_handle_pool, RENDERER_RESOURCE_TYPE_SHADER, stringify(RENDERER_RESOURCE_TYPE_SHADER));
    handle_pool_set_type(&rs->resource_handle_pool, RENDERER_RESOURCE_TYPE_PIPELINE, stringify(RENDERER_RESOURCE_TYPE_PIPELINE));
    VkResult res;

    info("Creating Vulkan instance and debug callback");

    VkApplicationInfo ai = {};
    ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ai.pApplicationName = "ZGAE";
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

    fptr_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (fptr_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    res = vkCreateDebugUtilsMessengerEXT(instance, &debug_ext_ci, NULL, &rs->debug_messenger);
    VERIFY_RES();

    info("Creating XCB Vulkan surface");
    linux_xcb_window_t* win = window_data;
    VkXcbSurfaceCreateInfoKHR xcbci = {};
    xcbci.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    xcbci.connection = linux_xcb_window_get_connection(win);
    xcbci.window = linux_xcb_window_get_handle(win);
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
    check(rs->graphics_queue_family_idx != (uint32_t)-1, "Couldn't find graphics queue family");
    if (rs->present_queue_family_idx == (uint32_t)-1)
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
    check(rs->present_queue_family_idx != (uint32_t)-1, "Couldn't find present queue family");
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
        rs->surface_format, rs->graphics_queue_family_idx, rs->present_queue_family_idx);

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

static void destroy_renderer_resources(VkDevice device, handle_pool_t* hp, renderer_resource_t* rrs, size_t rrs_n)
{
    for (size_t i = 0; i < rrs_n; ++i)
    {
        renderer_resource_t* rr = rrs + i;

        switch(rr->type)
        {
            case RENDERER_RESOURCE_TYPE_SHADER: {
                shader_t* s = &rr->shader;
                vkDestroyShaderModule(device, s->module, NULL);
                memf(s->input_layout);
                memf(s->constant_buffer.items);
            } break;
            
            case RENDERER_RESOURCE_TYPE_PIPELINE: {
            } break;

            case RENDERER_RESOURCE_TYPE_INVALID:
                error("Invalid resource in renderer resource list"); break;
        }

        handle_pool_return(hp, rr->handle);
    }
}

void renderer_destroy(renderer_state_t* rs)
{
    info("Destroying Vulkan renderer");
    VkDevice device = rs->device;
    destroy_renderer_resources(rs->device, &rs->resource_handle_pool, rs->da_resources, array_num(rs->da_resources));
    array_destroy(rs->da_resources);
    vkFreeCommandBuffers(device, rs->graphics_cmd_pool, 1, &rs->graphics_cmd_buffer);
    destroy_depth_buffer(device, &rs->depth_buffer);
    vkDestroyCommandPool(device, rs->graphics_cmd_pool, NULL);
    for (uint32_t i = 0; i < rs->swapchain_buffers_count; i++)
        vkDestroyImageView(device, rs->swapchain_buffers[i].view, NULL);
    vkDestroySwapchainKHR(device, rs->swapchain, NULL);
    vkDestroyDevice(device, NULL);
    fptr_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (fptr_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(rs->instance, "vkDestroyDebugUtilsMessengerEXT");
    vkDestroyDebugUtilsMessengerEXT(rs->instance, rs->debug_messenger, NULL);
    vkDestroyInstance(rs->instance, NULL);
    handle_pool_destroy(&rs->resource_handle_pool);
    memf(rs);
}

static renderer_resource_handle_t add_resource(handle_pool_t* hp, renderer_resource_t** da_resources, renderer_resource_t* res)
{
    renderer_resource_handle_t h = handle_pool_reserve(hp, res->type);
    res->handle = h;
    array_fill_and_set(*da_resources, handle_index(h), *res);
    return h;
}

static renderer_resource_t* get_resource(renderer_state_t* rs, renderer_resource_handle_t h)
{
    renderer_resource_t* rr = rs->da_resources + handle_index(h);
    check_slow(handle_type(h) == rr->type, "Handle points to resource of wrong type");
    return rr;
}

renderer_resource_handle_t renderer_load_shader(renderer_state_t* rs, const shader_intermediate_t* si)
{
    renderer_resource_t shader_res = {};
    shader_res.type = RENDERER_RESOURCE_TYPE_SHADER;
    shader_t* shader = &shader_res.shader;
    memcpy_alloc((void**)&shader->input_layout, si->input_layout, sizeof(shader_input_layout_item_t) * si->input_layout_num);
    shader->input_layout_num = si->input_layout_num;
    memcpy_alloc((void**)&shader->constant_buffer.items, si->constant_buffer.items, sizeof(shader_constant_buffer_item_t) * si->constant_buffer.items_num);
    shader->constant_buffer.items_num = si->constant_buffer.items_num;
    shader->type = si->type;

    VkShaderModuleCreateInfo smci = {};
    smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smci.pCode = (uint32_t*)si->source;
    smci.codeSize = si->source_size;

    VkResult res = vkCreateShaderModule(rs->device, &smci, NULL, &shader->module);
    VERIFY_RES();

    return add_resource(&rs->resource_handle_pool, &rs->da_resources, &shader_res);
}

VkFormat vk_format_from_shader_data_type(shader_data_type_t t)
{
    switch(t)
    {
        case SHADER_DATA_TYPE_MAT4: break;
        case SHADER_DATA_TYPE_VEC2: return VK_FORMAT_R32G32_SFLOAT;
        case SHADER_DATA_TYPE_VEC3: return VK_FORMAT_R32G32B32_SFLOAT;
        case SHADER_DATA_TYPE_VEC4: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case SHADER_DATA_TYPE_INVALID: break;
    }

    error("VkFormat unknown for shader data type %s", stringify(t));
    return -1;
}

VkShaderStageFlagBits vk_shader_stage_from_shader_type(shader_type_t t)
{
    switch(t)
    {
        case SHADER_TYPE_VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
        case SHADER_TYPE_FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
        default: break;
    }

    error("Trying to get VkShaderStageFlagBits from shader_type_t, but type isn't mapped.");
    return 0;
}

renderer_resource_handle_t renderer_load_pipeline(renderer_state_t* rs, const pipeline_intermediate_t* pi)
{
    renderer_resource_t pipeline_res = {};
    pipeline_res.type = RENDERER_RESOURCE_TYPE_PIPELINE;
    pipeline_t* pipeline = &pipeline_res.pipeline;
    VkResult res;

    // First we get all the data we need from the shader stages
    shader_input_layout_item_t* input_layout = NULL;
    uint32_t input_layout_num = 0;

    for (size_t i = 0; i < pi->shader_stages_num; ++i)
    {
        renderer_resource_handle_t rrh = pi->shader_stages[i];
        renderer_resource_t* rr = get_resource(rs, rrh);
        shader_t* s = &rr->shader;
        if (s->type == SHADER_TYPE_VERTEX)
        {
            check(s->input_layout_num > 0, "Vertex shader missing input layout.");
            input_layout = s->input_layout;
            input_layout_num = s->input_layout_num;
        }
    }

    check(input_layout_num > 0, "No shader with input layout in pipeline.");


    // Create vk descriptors that describe the input to vertex shader and the stride of the vertex data.
    VkVertexInputAttributeDescription* viad = mema_zero(sizeof(VkVertexInputAttributeDescription) * input_layout_num);

    uint32_t layout_offset = 0;
    for (uint32_t i = 0; i < input_layout_num; ++i)
    {
        viad[i].binding = 0;
        viad[i].location = i;
        viad[i].format = vk_format_from_shader_data_type(input_layout[i].type);
        viad[i].offset = layout_offset;
        layout_offset += shader_data_type_size(input_layout[i].type);
    }

    uint32_t stride = layout_offset;

    VkVertexInputBindingDescription vibd = {};
    vibd.binding = 0;
    vibd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vibd.stride = stride;

    VkPipelineVertexInputStateCreateInfo pvisci = {};
    pvisci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pvisci.vertexBindingDescriptionCount = 1;
    pvisci.pVertexBindingDescriptions = &vibd;
    pvisci.vertexAttributeDescriptionCount = input_layout_num;
    pvisci.pVertexAttributeDescriptions = viad;

    // Create vk uniform buffers for our constant buffers
    uint32_t num_constant_buffers = 0;
    uint32_t* arr_used_bindings;

    for (size_t i = 0; i < pi->shader_stages_num; ++i)
    {
        renderer_resource_handle_t rrh = pi->shader_stages[i];
        renderer_resource_t* rr = get_resource(rs, rrh);
        shader_t* s = &rr->shader;

        if (s->constant_buffer.items_num == 0)
            continue;
        
        for (uint32_t i = 0; i < array_num(arr_used_bindings); ++i)
            check(arr_used_bindings[i] != s->constant_buffer.binding, "In pipeline there are two shaders with same constant buffer binding num.");

        array_add(arr_used_bindings, s->constant_buffer.binding);
        num_constant_buffers += 1;
    }

    array_destroy(arr_used_bindings);
    pipeline->constant_buffers = mema_zero(sizeof(VkBuffer) * num_constant_buffers);
    pipeline->constant_buffers_num = 0;
    VkDescriptorSetLayoutBinding* constant_buffer_bindings = mema_zero(sizeof(VkDescriptorSetLayoutBinding) * num_constant_buffers);

    for (size_t i = 0; i < pi->shader_stages_num; ++i)
    {
        renderer_resource_handle_t rrh = pi->shader_stages[i];
        renderer_resource_t* rr = get_resource(rs, rrh);
        shader_t* s = &rr->shader;

        if (s->constant_buffer.items_num == 0)
            continue;

        uint32_t cb_size = 0;

        for (uint32_t j = 0; j < s->constant_buffer.items_num; ++j)
            cb_size += shader_data_type_size(s->constant_buffer.items[j].type);

        VkBufferCreateInfo cb_bci = {};
        cb_bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        cb_bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        cb_bci.size = cb_size;
        cb_bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        res = vkCreateBuffer(rs->device, &cb_bci, NULL, pipeline->constant_buffers + pipeline->constant_buffers_num);
        VERIFY_RES();

        constant_buffer_bindings[pipeline->constant_buffers_num].binding = s->constant_buffer.binding;
        constant_buffer_bindings[pipeline->constant_buffers_num].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        constant_buffer_bindings[pipeline->constant_buffers_num].descriptorCount = 1;
        constant_buffer_bindings[pipeline->constant_buffers_num].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        ++pipeline->constant_buffers_num;
    }

    VkDescriptorSetLayoutCreateInfo dslci = {};
    dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dslci.bindingCount = num_constant_buffers;
    dslci.pBindings = constant_buffer_bindings;

    VkDescriptorSetLayout dsl;
    res = vkCreateDescriptorSetLayout(rs->device, &dslci, NULL, &dsl);
    VERIFY_RES();

    VkPipelineLayoutCreateInfo plci = {};
    plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &dsl;

    VkPipelineLayout pl;
    res = vkCreatePipelineLayout(rs->device, &plci, NULL, &pl);
    VERIFY_RES();

    // TODO: add descriptor sets thingy here, vkUpdateDescriptorSets etc


    // Set which topology we want
    VkPipelineInputAssemblyStateCreateInfo piasci = {};
    piasci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    piasci.primitiveRestartEnable = VK_FALSE;
    piasci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


    // Rasteriser settings
    VkPipelineRasterizationStateCreateInfo prsci = {};
    prsci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    prsci.polygonMode = VK_POLYGON_MODE_FILL;
    prsci.cullMode = VK_CULL_MODE_BACK_BIT;
    prsci.frontFace = VK_FRONT_FACE_CLOCKWISE;
    prsci.depthClampEnable = VK_FALSE;
    prsci.rasterizerDiscardEnable = VK_FALSE;
    prsci.depthBiasEnable = VK_FALSE;
    prsci.depthBiasConstantFactor = 0;
    prsci.depthBiasClamp = 0;
    prsci.depthBiasSlopeFactor = 0;
    prsci.lineWidth = 1.0f;


    // Blending settings
    VkPipelineColorBlendStateCreateInfo pcbsci = {};
    pcbsci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    VkPipelineColorBlendAttachmentState pcbas[1];
    memset(pcbas, 0, sizeof(pcbas));
    pcbas[0].colorWriteMask = 0xf;
    pcbas[0].blendEnable = VK_FALSE;
    pcbas[0].alphaBlendOp = VK_BLEND_OP_ADD;
    pcbas[0].colorBlendOp = VK_BLEND_OP_ADD;
    pcbas[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    pcbas[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    pcbas[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pcbas[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pcbsci.attachmentCount = 1;
    pcbsci.pAttachments = pcbas;
    pcbsci.logicOpEnable = VK_FALSE;
    pcbsci.logicOp = VK_LOGIC_OP_NO_OP;
    pcbsci.blendConstants[0] = 1.0f;
    pcbsci.blendConstants[1] = 1.0f;
    pcbsci.blendConstants[2] = 1.0f;
    pcbsci.blendConstants[3] = 1.0f;


    // Multisampling settings
    VkPipelineMultisampleStateCreateInfo pmsci = {};
    pmsci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pmsci.rasterizationSamples = NUM_SAMPLES;
    pmsci.sampleShadingEnable = VK_FALSE;
    pmsci.alphaToCoverageEnable = VK_FALSE;
    pmsci.alphaToOneEnable = VK_FALSE;
    pmsci.minSampleShading = 0.0;


    // Dynamic state (like, which things can change with every command)
    VkDynamicState dynamic_state_enables[VK_DYNAMIC_STATE_RANGE_SIZE];
    memset(dynamic_state_enables, 0, sizeof(dynamic_state_enables));
    VkPipelineDynamicStateCreateInfo pdsci = {};
    pdsci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pdsci.pDynamicStates = dynamic_state_enables;


    // Viewport settings
    VkPipelineViewportStateCreateInfo pvpsci = {};
    pvpsci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pvpsci.viewportCount = 1;
    dynamic_state_enables[pdsci.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    pvpsci.scissorCount = 1;
    dynamic_state_enables[pdsci.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
    pvpsci.pScissors = NULL;
    pvpsci.pViewports = NULL;


    // Depth-stencil settings
    VkPipelineDepthStencilStateCreateInfo pdssci = {};
    pdssci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pdssci.depthTestEnable = VK_TRUE;
    pdssci.depthWriteEnable = VK_TRUE;
    pdssci.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pdssci.depthBoundsTestEnable = VK_FALSE;
    pdssci.minDepthBounds = 0;
    pdssci.maxDepthBounds = 0;
    pdssci.stencilTestEnable = VK_FALSE;
    pdssci.back.failOp = VK_STENCIL_OP_KEEP;
    pdssci.back.passOp = VK_STENCIL_OP_KEEP;
    pdssci.back.compareOp = VK_COMPARE_OP_ALWAYS;
    pdssci.back.compareMask = 0;
    pdssci.back.reference = 0;
    pdssci.back.depthFailOp = VK_STENCIL_OP_KEEP;
    pdssci.back.writeMask = 0;
    pdssci.front = pdssci.back;


    // Shader stage info
    VkPipelineShaderStageCreateInfo* pssci = mema_zero(sizeof(VkPipelineShaderStageCreateInfo) * pi->shader_stages_num);

    for (size_t i = 0; i < pi->shader_stages_num; ++i)
    {
        renderer_resource_handle_t rrh = pi->shader_stages[i];
        renderer_resource_t* rr = get_resource(rs, rrh);
        shader_t* s = &rr->shader;

        pssci[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pssci[i].stage = vk_shader_stage_from_shader_type(s->type);
        pssci[i].pName = "main";
        pssci[i].module = s->module;
    }


    // Create actual vk pipeline
    VkGraphicsPipelineCreateInfo pci = {};
    pci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pci.layout = pl;
    pci.pVertexInputState = &pvisci;
    pci.pInputAssemblyState = &piasci;
    pci.pRasterizationState = &prsci;
    pci.pColorBlendState = &pcbsci;
    pci.pTessellationState = NULL;
    pci.pMultisampleState = &pmsci;
    pci.pDynamicState = &pdsci;
    pci.pViewportState = &pvpsci;
    pci.pDepthStencilState = &pdssci;
    pci.pStages = pssci;
    pci.stageCount = pi->shader_stages_num;
    pci.renderPass = NULL;//render_pass;
    pci.subpass = 0;

    res = vkCreateGraphicsPipelines(rs->device, VK_NULL_HANDLE, 1, &pci, NULL, &pipeline->vk_handle);
    VERIFY_RES();

    return add_resource(&rs->resource_handle_pool, &rs->da_resources, &pipeline_res);
}