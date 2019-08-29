#include "renderer_backend.h"
#include <vulkan/vulkan.h>
#include "linux_xcb_window.h"
#include "memory.h"
#include "debug.h"
#include "window_types.h"
#include <string.h>
#include "mesh_types.h"
#include "str.h"
#include "render_resource_types.h"
#include "render_resource.h"

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT
#define VERIFY_RES() check(res == VK_SUCCESS, "Vulkan error (VkResult is %s)", res)

#define MAX_FRAMES_IN_FLIGHT 2

struct SwapchainBuffer
{
    VkImage image;
    VkImageView view;
    VkFramebuffer framebuffer;
};

struct DepthBuffer
{
    VkFormat format;
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
};

struct RenderBackendShader
{
    VkShaderModule module;
};

struct PipelineConstantBuffer
{
    VkBuffer vk_handle[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory memory[MAX_FRAMES_IN_FLIGHT];
    u32 binding;
    u32 size;
    u32 allocated_size;
};

struct RenderBackendPipeline
{
    PipelineConstantBuffer* constant_buffers;
    VkDescriptorSet* constant_buffer_descriptor_sets[MAX_FRAMES_IN_FLIGHT];
    VkDescriptorSetLayout constant_buffer_descriptor_set_layout;
    u32 constant_buffers_num;
    VkPipeline vk_handle;
    VkPipelineLayout layout;
};

struct RenderBackendMesh
{
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
    u32 indices_num;
};

struct RendererBackend
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
    Vec2u swapchain_size;
    u32 current_frame;
    SwapchainBuffer* swapchain_buffers;
    VkFence image_in_flight_fences[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
    u32 swapchain_buffers_num;
    u32 graphics_queue_family_idx;
    VkQueue graphics_queue;
    u32 present_queue_family_idx;
    VkQueue present_queue;
    VkCommandPool graphics_cmd_pool;
    VkCommandBuffer* graphics_cmd_buffers;
    u32 graphics_cmd_buffers_num;
    DepthBuffer depth_buffer;
    VkDescriptorPool descriptor_pool_uniform_buffer;
    VkRenderPass render_pass;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_message_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* user_data)
{
    (void)type;
    (void)user_data;

    if (severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        info("(From Vulkan) %s", data->pMessage);
        return VK_FALSE;
    }

    static const char* non_errors[] = {
        "VUID-VkSwapchainCreateInfoKHR-imageExtent-01274",
        "UNASSIGNED-CoreValidation-DrawState-SwapchainTooManyImages"
    };

    static const bool non_errors_suppress[] = {
        false,
        true
    };

    static const char* non_errors_reasons[] = {
        "A race condition between swapchain creation and window resizing happening at arbitrary times makes it impossible to avoid this error. The window will later send a new resize callback, making everything fine.",
        "Due to Vulkan bug (https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/issues/1670)."
    };

    i32 non_error_idx = str_eql_arr((const char*)data->pMessageIdName, non_errors, sizeof(non_errors)/sizeof(char*));

    if (non_error_idx != -1)
    {
        if (non_errors_suppress[non_error_idx])
            return VK_FALSE;

        info("The next Vulkan message occured as an error in Vulkan, it is hard-coded to not trigger an error with this reason: \"%s\"", non_errors_reasons[non_error_idx]);
        info("(From Vulkan) %s", data->pMessage);
        return VK_FALSE;
    }

    info("Error occured in Vulkan, error message id is: %s", data->pMessageIdName);
    error("(From Vulkan) %s", data->pMessage);

    return VK_FALSE;
}

static void create_swapchain(
    VkSwapchainKHR* out_current_swapchain, SwapchainBuffer** out_sc_bufs, u32* out_sc_bufs_num, Vec2u size,
    VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface, VkFormat format,
    u32 graphics_queue_family_idx, u32 present_queue_family_idx)
{
    info("Creating swapchain with size %dx%d", size.x, size.y);
    VkResult res;

    VkSurfaceCapabilitiesKHR surface_capabilities;
    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_capabilities);
    VERIFY_RES();

    VkSurfaceTransformFlagBitsKHR pre_transform = 
        surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
        ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
        : surface_capabilities.currentTransform;


    // Choose present mode
    u32 present_modes_count;
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_modes_count, NULL);
    VERIFY_RES();
    VkPresentModeKHR* present_modes = mema_tn(VkPresentModeKHR, present_modes_count);
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_modes_count, present_modes);
    VERIFY_RES();
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR; 
    for (u32 i = 0; i < present_modes_count; ++i)
    {
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    memf(present_modes);


    // Create swapchain
    VkSwapchainCreateInfoKHR scci = {};
    scci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    scci.surface = surface;
    scci.minImageCount = surface_capabilities.minImageCount;
    scci.imageFormat = format;
    scci.imageExtent.width = size.x;
    scci.imageExtent.height = size.y;
    scci.preTransform = pre_transform;
    scci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    scci.imageArrayLayers = 1;
    scci.presentMode = present_mode;
    scci.oldSwapchain = *out_current_swapchain;
    scci.clipped = 1;
    scci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    scci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    scci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    u32 queue_family_indicies[] = {graphics_queue_family_idx, present_queue_family_idx};

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

    u32 swapchain_image_count;
    res = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, NULL);
    VERIFY_RES();
    check(swapchain_image_count > 0, "Swapchain contains no images");
    VkImage* swapchain_images = mema_tn(VkImage, swapchain_image_count);
    res = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images);
    VERIFY_RES();

    info("Creating %d buffers for swapchain", swapchain_image_count);
    SwapchainBuffer* old_bufs = *out_sc_bufs;
    u32 old_bufs_num = *out_sc_bufs_num;

    for (u32 i = 0; i < old_bufs_num; i++)
    {
        vkDestroyImageView(device, old_bufs[i].view, NULL);
        vkDestroyFramebuffer(device, old_bufs[i].framebuffer, NULL);
    }

    SwapchainBuffer* bufs = mema_zero_tn(SwapchainBuffer, swapchain_image_count);

    for (u32 i = 0; i < swapchain_image_count; ++i)
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
    *out_sc_bufs_num = swapchain_image_count;
}

static void create_framebuffers(VkDevice device, SwapchainBuffer* swapchain_buffers, u32 swapchain_buffers_num, DepthBuffer* depth_buffer, VkRenderPass render_pass, Vec2u swapchain_size)
{
    VkImageView framebuffer_attachments[2];
    framebuffer_attachments[1] = depth_buffer->view;

    VkFramebufferCreateInfo fbci = {};
    fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbci.renderPass = render_pass;
    fbci.attachmentCount = 2;
    fbci.pAttachments = framebuffer_attachments;
    fbci.width = swapchain_size.x;
    fbci.height = swapchain_size.y;
    fbci.layers = 1;

    for (u32 i = 0; i < swapchain_buffers_num; ++i)
    {
        framebuffer_attachments[0] = swapchain_buffers[i].view;
        VkResult res = vkCreateFramebuffer(device, &fbci, NULL, &swapchain_buffers[i].framebuffer);
        VERIFY_RES();
    }
}

static u32 memory_type_from_properties(u32 req_memory_type, VkPhysicalDeviceMemoryProperties* memory_properties, VkMemoryPropertyFlags memory_requirement_mask)
{
    for (u32 i = 0; i < memory_properties->memoryTypeCount; ++i)
    {
        if ((req_memory_type & (1 << i)) && (memory_properties->memoryTypes[i].propertyFlags & memory_requirement_mask) == memory_requirement_mask)
        {
            return i;
        }
    }

    return -1;
}

static void destroy_depth_buffer(VkDevice device, DepthBuffer* depth_buffer)
{
    info("Destroying depth buffer");
    vkDestroyImageView(device, depth_buffer->view, NULL);
    vkDestroyImage(device, depth_buffer->image, NULL);
    vkFreeMemory(device, depth_buffer->memory, NULL);
    memzero(depth_buffer, sizeof(DepthBuffer));
}

static void create_depth_buffer(DepthBuffer* out_depth_buffer, VkDevice device, VkPhysicalDevice gpu, VkPhysicalDeviceMemoryProperties* memory_properties, Vec2u size)
{
    info("Creating depth buffer");
    DepthBuffer depth_buffer = {};
    depth_buffer.format = VK_FORMAT_D16_UNORM;

    VkImageCreateInfo depth_ici = {};
    VkFormatProperties depth_format_props;
    vkGetPhysicalDeviceFormatProperties(gpu, depth_buffer.format, &depth_format_props);
    if (depth_format_props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        depth_ici.tiling = VK_IMAGE_TILING_LINEAR;
    else if (depth_format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        depth_ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    else
        error("VK_FORMAT_D16_UNORM unsupported");

    depth_ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depth_ici.imageType = VK_IMAGE_TYPE_2D;
    depth_ici.format = depth_buffer.format;
    depth_ici.extent.width = size.x;
    depth_ici.extent.height = size.y;
    depth_ici.extent.depth = 1;
    depth_ici.mipLevels = 1;
    depth_ici.arrayLayers = 1;

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

    check(depth_mai.memoryTypeIndex != (u32)-1, "Failed to find memory type for depth buffer");
    res = vkAllocateMemory(device, &depth_mai, NULL, &depth_buffer.memory);
    VERIFY_RES();
    res = vkBindImageMemory(device, depth_buffer.image, depth_buffer.memory, 0);
    VERIFY_RES();

    VkImageViewCreateInfo depth_ivci = {};
    depth_ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depth_ivci.image = VK_NULL_HANDLE;
    depth_ivci.format = depth_buffer.format;
    depth_ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth_ivci.subresourceRange.levelCount = 1;
    depth_ivci.subresourceRange.layerCount = 1;
    depth_ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depth_ivci.image = depth_buffer.image;
    res = vkCreateImageView(device, &depth_ivci, NULL, &depth_buffer.view);
    VERIFY_RES();
    *out_depth_buffer = depth_buffer;
}

static void create_renderpass(VkDevice device, VkFormat surface_format, VkFormat depth_format, VkRenderPass* out_render_pass)
{
    info("Creating render pass");
    VkAttachmentDescription attachments[2];
    memzero(attachments, sizeof(VkAttachmentDescription) * 2);
    attachments[0].format = surface_format;
    attachments[0].samples = NUM_SAMPLES;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments[1].format = depth_format;
    attachments[1].samples = NUM_SAMPLES;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_reference = {};
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {};
    depth_reference.attachment = 1;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_reference;
    subpass.pDepthStencilAttachment = &depth_reference;

    VkRenderPassCreateInfo rpci = {};
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = 2;
    rpci.pAttachments = attachments;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    VkResult res = vkCreateRenderPass(device, &rpci, NULL, out_render_pass);
    VERIFY_RES();
}

static void destroy_swapchain(RendererBackend* rbs)
{
    info("Destroying swapchain");
    VkDevice d = rbs->device;

    for (u32 i = 0; i < rbs->swapchain_buffers_num; i++)
    {
        vkDestroyImageView(d, rbs->swapchain_buffers[i].view, NULL);
        vkDestroyFramebuffer(d, rbs->swapchain_buffers[i].framebuffer, NULL);
    }

    memf(rbs->swapchain_buffers);
    rbs->swapchain_buffers = NULL;
    rbs->swapchain_buffers_num = 0;

    vkDestroySwapchainKHR(d, rbs->swapchain, NULL);
    rbs->swapchain = NULL;
}

static void destroy_renderpass(RendererBackend* rbs)
{
    info("Destroying render pass");
    VkDevice d = rbs->device;
    vkDestroyRenderPass(d, rbs->render_pass, NULL);
    rbs->render_pass = NULL;
}

static void destroy_surface_size_dependent_resources(RendererBackend* rbs)
{
    destroy_swapchain(rbs);
    destroy_renderpass(rbs);
    destroy_depth_buffer(rbs->device, &rbs->depth_buffer);
}

static void create_surface_size_dependent_resources(RendererBackend* rbs)
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(rbs->gpu, rbs->surface, &surface_capabilities);
    VERIFY_RES();
    check(surface_capabilities.currentExtent.width != (u32)-1, "Couldn't get surface size");
    rbs->swapchain_size.x = surface_capabilities.currentExtent.width;
    rbs->swapchain_size.y = surface_capabilities.currentExtent.height;
    
    create_depth_buffer(&rbs->depth_buffer, rbs->device, rbs->gpu, &rbs->gpu_memory_properties, rbs->swapchain_size);

    create_renderpass(rbs->device, rbs->surface_format, rbs->depth_buffer.format, &rbs->render_pass);

    create_swapchain(
        &rbs->swapchain, &rbs->swapchain_buffers, &rbs->swapchain_buffers_num, rbs->swapchain_size,
        rbs->gpu, rbs->device, rbs->surface, rbs->surface_format,
        rbs->graphics_queue_family_idx, rbs->present_queue_family_idx);

    create_framebuffers(rbs->device, rbs->swapchain_buffers, rbs->swapchain_buffers_num, &rbs->depth_buffer, rbs->render_pass, rbs->swapchain_size);

    rbs->current_frame = 0;
}

static void recreate_surface_size_dependent_resources(RendererBackend* rbs)
{
    destroy_surface_size_dependent_resources(rbs);
    create_surface_size_dependent_resources(rbs);
}

static VkFormat choose_surface_format(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
    VkResult res;
    u32 num_supported_surface_formats;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &num_supported_surface_formats, NULL);
    VERIFY_RES();
    VkSurfaceFormatKHR* supported_surface_formats = mema_tn(VkSurfaceFormatKHR, num_supported_surface_formats);
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &num_supported_surface_formats, supported_surface_formats);
    VERIFY_RES();

    // This means we can choose ourselves.
    if (num_supported_surface_formats == 1 && supported_surface_formats[0].format == VK_FORMAT_UNDEFINED)
    {
        memf(supported_surface_formats);
        return VK_FORMAT_R8G8B8A8_UNORM;
    }

    check(num_supported_surface_formats > 0, "No supported surface formats found");

    for (u32 i = 0; i < num_supported_surface_formats; ++i)
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

static VkPhysicalDevice choose_gpu(VkPhysicalDevice* gpus, u32 num_gpus)
{
    check(num_gpus > 0, "Trying to select among 0 GPUs");

    if (num_gpus == 1)
        return gpus[0];

    for (u32 i = 0; i < num_gpus; ++i)
    {
        VkPhysicalDeviceProperties gpu_properties;
        vkGetPhysicalDeviceProperties(gpus[i], &gpu_properties);

        if (gpu_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            return gpus[i];
    }

    return gpus[0];
}

typedef VkResult (*fptr_vkCreateDebugUtilsMessengerEXT)(VkInstance, VkDebugUtilsMessengerCreateInfoEXT*, VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*);
typedef void (*fptr_vkDestroyDebugUtilsMessengerEXT)(VkInstance, VkDebugUtilsMessengerEXT, VkAllocationCallbacks*);

RendererBackend* renderer_backend_create(WindowType window_type, void* window_data)
{
    info("Creating Vulkan render");

    check(window_type == WINDOW_TYPE_XCB, "window_type must be WindowType::Xcb");
    RendererBackend* rbs = mema_zero_t(RendererBackend);
    VkResult res;

    info("Creating Vulkan instance and debug callback");

    VkApplicationInfo ai = {};
    ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ai.pApplicationName = "ZGAE";
    ai.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    ai.apiVersion = VK_API_VERSION_1_0;

    char* validation_layer_name = "VK_LAYER_LUNARG_standard_validation";
    u32 available_layers_num;
    res = vkEnumerateInstanceLayerProperties(&available_layers_num, NULL);
    VERIFY_RES();
    VkLayerProperties* available_layers = mema_tn(VkLayerProperties, available_layers_num);
    res = vkEnumerateInstanceLayerProperties(&available_layers_num, available_layers);
    VERIFY_RES();
    bool validation_layer_available = false;
    for (u32 i = 0; i < available_layers_num; ++i)
    {
        if (str_eql(validation_layer_name, available_layers[i].layerName))
        {
            validation_layer_available = true;
            break;
        }
    }
    memf(available_layers);

    check(validation_layer_available, "Validation layer not available!");
    VkDebugUtilsMessengerCreateInfoEXT debug_ext_ci = {};
    debug_ext_ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_ext_ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_ext_ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_ext_ci.pfnUserCallback = vulkan_debug_message_callback;

    char* validation_layers[] = {validation_layer_name};
    char* extensions[] = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XCB_SURFACE_EXTENSION_NAME};
    
    VkInstanceCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &ai;
    ici.ppEnabledExtensionNames = extensions;
    ici.enabledExtensionCount = sizeof(extensions)/sizeof(char*);
    ici.ppEnabledLayerNames = validation_layers;
    ici.enabledLayerCount = sizeof(validation_layers)/sizeof(char*);
    ici.pNext = &debug_ext_ci;
    res = vkCreateInstance(&ici, NULL, &rbs->instance);
    VERIFY_RES();
    VkInstance instance = rbs->instance;

    fptr_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (fptr_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    res = vkCreateDebugUtilsMessengerEXT(instance, &debug_ext_ci, NULL, &rbs->debug_messenger);
    VERIFY_RES();

    info("Creating XCB Vulkan surface");
    XcbWindow* win = (XcbWindow*)window_data;
    VkXcbSurfaceCreateInfoKHR xcbci = {};
    xcbci.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    xcbci.connection = linux_xcb_window_get_connection(win);
    xcbci.window = linux_xcb_window_get_handle(win);
    res = vkCreateXcbSurfaceKHR(instance, &xcbci, NULL, &rbs->surface);
    VERIFY_RES();
    VkSurfaceKHR surface = rbs->surface;

    info("Selecting GPU and fetching properties");
    u32 num_available_gpus = 0;
    res = vkEnumeratePhysicalDevices(instance, &num_available_gpus, NULL);
    check(res == VK_SUCCESS || res == VK_INCOMPLETE, "Failed enumerating GPUs");
    check(num_available_gpus > 0, "No GPUS found");
    VkPhysicalDevice* available_gpus = mema_tn(VkPhysicalDevice, num_available_gpus);
    res = vkEnumeratePhysicalDevices(instance, &num_available_gpus, available_gpus);
    VERIFY_RES();
    rbs->gpu = choose_gpu(available_gpus, num_available_gpus);
    VkPhysicalDevice gpu = rbs->gpu;
    memf(available_gpus);
    vkGetPhysicalDeviceProperties(rbs->gpu, &rbs->gpu_properties);
    vkGetPhysicalDeviceMemoryProperties(rbs->gpu, &rbs->gpu_memory_properties);
    info("Selected GPU %s", rbs->gpu_properties.deviceName);

    info("Finding GPU graphics and present queue families");
    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, NULL);
    VkQueueFamilyProperties* queue_family_props = mema_tn(VkQueueFamilyProperties, queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, queue_family_props);
    
    VkBool32* queues_with_present_support = mema_tn(VkBool32, queue_family_count);
    for (u32 i = 0; i < queue_family_count; ++i)
    {
        res = vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &queues_with_present_support[i]);
        VERIFY_RES();
    }

    rbs->graphics_queue_family_idx = -1;
    rbs->present_queue_family_idx = -1;

    // We first try to find a queue family with both graphics and present capabilities,
    // if it fails we try to look for two separate queue families
    for (u32 i = 0; i < queue_family_count; ++i)
    {
        if (queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            rbs->graphics_queue_family_idx = i;

            if (queues_with_present_support[i] == VK_TRUE)
            {
                rbs->present_queue_family_idx = i;
                info("Found combined queue family");
                break;
            }
        }
    }
    check(rbs->graphics_queue_family_idx != (u32)-1, "Couldn't find graphics queue family");
    if (rbs->present_queue_family_idx == (u32)-1)
    {
        for (u32 i = 0; i < queue_family_count; ++i)
        {
            if (queues_with_present_support[i] == VK_TRUE)
            {
                rbs->present_queue_family_idx = i;
                info("Found two separate queue families");
                break;
            }
        }
    }
    check(rbs->present_queue_family_idx != (u32)-1, "Couldn't find present queue family");
    memf(queues_with_present_support);
    memf(queue_family_props);

    info("Creating Vulkan logical device");
    VkDeviceQueueCreateInfo dqci = {};
    dqci.queueFamilyIndex = rbs->graphics_queue_family_idx;
    dqci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    dqci.queueCount = 1;
    f32 queue_priorities[] = {0.0};
    dqci.pQueuePriorities = queue_priorities;

    char* device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo dci = {};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &dqci;
    dci.ppEnabledExtensionNames = device_extensions;
    dci.enabledExtensionCount = sizeof(device_extensions)/sizeof(char*);

    res = vkCreateDevice(gpu, &dci, NULL, &rbs->device);
    VERIFY_RES();
    VkDevice device = rbs->device;

    rbs->surface_format = choose_surface_format(gpu, surface);
    info("Chose surface VkFormat: %d", rbs->surface_format);
    
    create_surface_size_dependent_resources(rbs);

    info("Creating graphics and present queues");
    vkGetDeviceQueue(device, rbs->graphics_queue_family_idx, 0, &rbs->graphics_queue);
    if (rbs->graphics_queue_family_idx == rbs->present_queue_family_idx)
        rbs->present_queue = rbs->graphics_queue;
    else
        vkGetDeviceQueue(device, rbs->present_queue_family_idx, 0, &rbs->present_queue);

    info("Creating graphics queue command pool and command buffer");
    VkCommandPoolCreateInfo cpci = {};
    cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpci.queueFamilyIndex = rbs->graphics_queue_family_idx;
    cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    res = vkCreateCommandPool(device, &cpci, NULL, &rbs->graphics_cmd_pool);
    VERIFY_RES();

    rbs->graphics_cmd_buffers_num = rbs->swapchain_buffers_num;
    rbs->graphics_cmd_buffers = mema_zero_tn(VkCommandBuffer, rbs->graphics_cmd_buffers_num);
    VkCommandBufferAllocateInfo cbai = {};
    cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbai.commandPool = rbs->graphics_cmd_pool;
    cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandBufferCount = rbs->graphics_cmd_buffers_num;
    res = vkAllocateCommandBuffers(device, &cbai, rbs->graphics_cmd_buffers);
    VERIFY_RES();

    info("Creating descriptor pools");
    VkDescriptorPoolSize dps[1];
    dps[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dps[0].descriptorCount = 10;

    VkDescriptorPoolCreateInfo dpci = {};
    dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    dpci.maxSets = 10;
    dpci.poolSizeCount = 1;
    dpci.pPoolSizes = dps;

    res = vkCreateDescriptorPool(device, &dpci, NULL, &rbs->descriptor_pool_uniform_buffer);
    VERIFY_RES();

    info("Creating semaphores and fences for frame syncronisation.");

    VkSemaphoreCreateInfo iasci = {};
    iasci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fci = {};
    fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        res = vkCreateSemaphore(device, &iasci, NULL, &rbs->image_available_semaphores[i]);
        VERIFY_RES();
        res = vkCreateSemaphore(device, &iasci, NULL, &rbs->render_finished_semaphores[i]);
        VERIFY_RES();
        res = vkCreateFence(device, &fci, NULL, &rbs->image_in_flight_fences[i]);
        VERIFY_RES();
    }

    return rbs;
}

void renderer_backend_destroy_shader(RendererBackend* rbs, RenderBackendShader* s)
{
    vkDestroyShaderModule(rbs->device, s->module, NULL);
    memf(s);
}

void renderer_backend_destroy_pipeline(RendererBackend* rbs, RenderBackendPipeline* p)
{
    for (u32 frame_idx = 0; frame_idx < MAX_FRAMES_IN_FLIGHT; ++frame_idx)
    {
        vkFreeDescriptorSets(rbs->device, rbs->descriptor_pool_uniform_buffer, 1, p->constant_buffer_descriptor_sets[frame_idx]);
        memf(p->constant_buffer_descriptor_sets[frame_idx]);
        for (u32 cb_idx = 0; cb_idx < p->constant_buffers_num; ++cb_idx)
        {
            vkDestroyBuffer(rbs->device, p->constant_buffers[cb_idx].vk_handle[frame_idx], NULL);
            vkFreeMemory(rbs->device, p->constant_buffers[cb_idx].memory[frame_idx], NULL);
        }
    }

    vkDestroyPipelineLayout(rbs->device, p->layout, NULL);
    vkDestroyDescriptorSetLayout(rbs->device, p->constant_buffer_descriptor_set_layout, NULL);
    vkDestroyPipeline(rbs->device, p->vk_handle, NULL);
    memf(p->constant_buffers);
    memf(p);
}

void renderer_backend_destroy_mesh(RendererBackend* rbs, RenderBackendMesh* g)
{
    vkDestroyBuffer(rbs->device, g->vertex_buffer, NULL);
    vkFreeMemory(rbs->device, g->vertex_buffer_memory, NULL);
    vkDestroyBuffer(rbs->device, g->index_buffer, NULL);
    vkFreeMemory(rbs->device, g->index_buffer_memory, NULL);
    memf(g);
}

void renderer_backend_destroy(RendererBackend* rbs)
{
    info("Destroying Vulkan render backend");

    VkDevice d = rbs->device;

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkWaitForFences(d, 1, &rbs->image_in_flight_fences[i], VK_TRUE, UINT64_MAX);
    }

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(d, rbs->render_finished_semaphores[i], NULL);
        vkDestroySemaphore(d, rbs->image_available_semaphores[i], NULL);
        vkDestroyFence(d, rbs->image_in_flight_fences[i], NULL);
    }

    destroy_surface_size_dependent_resources(rbs);
    vkDestroyDescriptorPool(d, rbs->descriptor_pool_uniform_buffer, NULL);

    vkFreeCommandBuffers(d, rbs->graphics_cmd_pool, rbs->graphics_cmd_buffers_num, rbs->graphics_cmd_buffers);
    memf(rbs->graphics_cmd_buffers);
    vkDestroyCommandPool(d, rbs->graphics_cmd_pool, NULL);

    vkDestroyDevice(d, NULL);
    fptr_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (fptr_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(rbs->instance, "vkDestroyDebugUtilsMessengerEXT");
    vkDestroyDebugUtilsMessengerEXT(rbs->instance, rbs->debug_messenger, NULL);
    vkDestroyInstance(rbs->instance, NULL);
    
    memf(rbs);
}

RenderBackendShader* renderer_backend_create_shader(RendererBackend* rbs, char* source, u32 source_size)
{
    RenderBackendShader* shader = mema_zero_t(RenderBackendShader);

    VkShaderModuleCreateInfo smci = {};
    smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smci.pCode = (u32*)source;
    smci.codeSize = source_size;

    VkResult res = vkCreateShaderModule(rbs->device, &smci, NULL, &shader->module);
    VERIFY_RES();

    return shader;
}

static VkFormat vk_format_from_shader_data_type(ShaderDataType t)
{
    switch(t)
    {
        case SHADER_DATA_TYPE_MAT4: break;
        case SHADER_DATA_TYPE_VEC2: return VK_FORMAT_R32G32_SFLOAT;
        case SHADER_DATA_TYPE_VEC3: return VK_FORMAT_R32G32B32_SFLOAT;
        case SHADER_DATA_TYPE_VEC4: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case SHADER_DATA_TYPE_INVALID: break;
    }

    error("VkFormat unknown for shader data type %d", t);
}

static VkShaderStageFlagBits vk_shader_stage_from_shader_type(ShaderType t)
{
    switch(t)
    {
        case SHADER_TYPE_VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
        case SHADER_TYPE_FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
        default: break;
    }

    error("Trying to get VkShaderStageFlagBits from ShaderType, but type isn't mapped.");
}

RenderBackendPipeline* renderer_backend_create_pipeline(RendererBackend* rbs,
    RenderBackendShader** shader_stages, ShaderType* shader_stages_types, u32 shader_stages_num,
    ShaderDataType* vertex_input_types, u32 vertex_input_types_num,
    u32* constant_buffer_sizes, u32* constant_buffer_binding_indices, u32 constant_buffers_num)
{
    RenderBackendPipeline* pipeline = mema_zero_t(RenderBackendPipeline);
    VkResult res;

    // Create vk descriptors that describe the input to vertex shader and the stride of the vertex data.
    VkVertexInputAttributeDescription* viad = mema_zero_tn(VkVertexInputAttributeDescription, vertex_input_types_num);

    u32 layout_offset = 0;
    for (u32 i = 0; i < vertex_input_types_num; ++i)
    {
        viad[i].binding = 0;
        viad[i].location = i;
        viad[i].format = vk_format_from_shader_data_type(vertex_input_types[i]);
        viad[i].offset = layout_offset;
        layout_offset += shader_data_type_size(vertex_input_types[i]);
    }

    u32 stride = layout_offset;

    VkVertexInputBindingDescription vibd = {};
    vibd.binding = 0;
    vibd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vibd.stride = stride;

    VkPipelineVertexInputStateCreateInfo pvisci = {};
    pvisci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pvisci.vertexBindingDescriptionCount = 1;
    pvisci.pVertexBindingDescriptions = &vibd;
    pvisci.vertexAttributeDescriptionCount = vertex_input_types_num;
    pvisci.pVertexAttributeDescriptions = viad;

    // Create vk uniform buffers for our constant buffers
    pipeline->constant_buffers = mema_zero_tn(PipelineConstantBuffer, constant_buffers_num);
    pipeline->constant_buffers_num = constant_buffers_num;
    VkDescriptorSetLayoutBinding* constant_buffer_bindings = mema_zero_tn(VkDescriptorSetLayoutBinding, constant_buffers_num);

    for (u32 cb_idx = 0; cb_idx < constant_buffers_num; ++cb_idx)
    {
        PipelineConstantBuffer* cb = pipeline->constant_buffers + cb_idx;
        cb->binding = constant_buffer_binding_indices[cb_idx];
        cb->size = constant_buffer_sizes[cb_idx];

        VkBufferCreateInfo cb_bci = {};
        cb_bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        cb_bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        cb_bci.size = cb->size;
        cb_bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            res = vkCreateBuffer(rbs->device, &cb_bci, NULL, &cb->vk_handle[i]);
            VERIFY_RES();
        
            VkMemoryRequirements uniform_buffer_mem_reqs;
            vkGetBufferMemoryRequirements(rbs->device, cb->vk_handle[i], &uniform_buffer_mem_reqs);

            VkMemoryAllocateInfo uniform_buffer_mai = {};
            uniform_buffer_mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            uniform_buffer_mai.allocationSize = uniform_buffer_mem_reqs.size;
            cb->allocated_size = uniform_buffer_mem_reqs.size;

            uniform_buffer_mai.memoryTypeIndex = memory_type_from_properties(uniform_buffer_mem_reqs.memoryTypeBits, &rbs->gpu_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            check(uniform_buffer_mai.memoryTypeIndex != (u32)-1, "Failed finding memory type for uniform buffer");

            res = vkAllocateMemory(rbs->device, &uniform_buffer_mai, NULL, &cb->memory[i]);
            VERIFY_RES();
            res = vkBindBufferMemory(rbs->device, cb->vk_handle[i], cb->memory[i], 0);
            VERIFY_RES();
        }

        constant_buffer_bindings[cb_idx].binding = cb->binding;
        constant_buffer_bindings[cb_idx].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        constant_buffer_bindings[cb_idx].descriptorCount = 1;
        constant_buffer_bindings[cb_idx].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    }

    VkDescriptorSetLayoutCreateInfo dslci = {};
    dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dslci.bindingCount = constant_buffers_num;
    dslci.pBindings = constant_buffer_bindings;

    res = vkCreateDescriptorSetLayout(rbs->device, &dslci, NULL, &pipeline->constant_buffer_descriptor_set_layout);
    VERIFY_RES();

    memf(constant_buffer_bindings);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        pipeline->constant_buffer_descriptor_sets[i] = mema_zero_tn(VkDescriptorSet, constant_buffers_num);
        VkDescriptorSetAllocateInfo dsai = {};
        dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        dsai.pNext = NULL;
        dsai.descriptorPool = rbs->descriptor_pool_uniform_buffer;
        dsai.descriptorSetCount = constant_buffers_num;
        dsai.pSetLayouts = &pipeline->constant_buffer_descriptor_set_layout;
        res = vkAllocateDescriptorSets(rbs->device, &dsai, pipeline->constant_buffer_descriptor_sets[i]);
        VERIFY_RES();
    }

    VkPipelineLayoutCreateInfo plci = {};
    plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &pipeline->constant_buffer_descriptor_set_layout;

    res = vkCreatePipelineLayout(rbs->device, &plci, NULL, &pipeline->layout);
    VERIFY_RES();


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
    prsci.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
    VkPipelineShaderStageCreateInfo* pssci = mema_zero_tn(VkPipelineShaderStageCreateInfo, shader_stages_num);

    for (u32 i = 0; i < shader_stages_num; ++i)
    {
        pssci[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pssci[i].stage = vk_shader_stage_from_shader_type(shader_stages_types[i]);
        pssci[i].pName = "main";
        pssci[i].module = shader_stages[i]->module;
    }


    // Create actual vk pipeline
    VkGraphicsPipelineCreateInfo pci = {};
    pci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pci.layout = pipeline->layout;
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
    pci.stageCount = shader_stages_num;
    pci.renderPass = rbs->render_pass;
    pci.subpass = 0;

    res = vkCreateGraphicsPipelines(rbs->device, VK_NULL_HANDLE, 1, &pci, NULL, &pipeline->vk_handle);
    VERIFY_RES();

    memf(viad);
    memf(pssci);

    return pipeline;
}

RenderBackendMesh* renderer_backend_create_mesh(RendererBackend* rbs, Mesh* m)
{
    VkResult res;


    // Vertexbuffer
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

    {
        VkBufferCreateInfo vertex_bci = {};
        vertex_bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertex_bci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vertex_bci.size = sizeof(MeshVertex) * m->vertices_num;
        vertex_bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
        res = vkCreateBuffer(rbs->device, &vertex_bci, NULL, &vertex_buffer);
        VERIFY_RES();
    
        VkMemoryRequirements vertex_buffer_mr;
        vkGetBufferMemoryRequirements(rbs->device, vertex_buffer, &vertex_buffer_mr);
        VkMemoryAllocateInfo vertex_buffer_mai = {};
        vertex_buffer_mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        vertex_buffer_mai.allocationSize = vertex_buffer_mr.size;
        vertex_buffer_mai.memoryTypeIndex = memory_type_from_properties(vertex_buffer_mr.memoryTypeBits, &rbs->gpu_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        check(vertex_buffer_mai.memoryTypeIndex != (u32)-1, "Couldn't find memory of correct type.");
    
        res = vkAllocateMemory(rbs->device, &vertex_buffer_mai, NULL, &vertex_buffer_memory);
        VERIFY_RES();
    
        u8* vertex_buffer_memory_data;
        res = vkMapMemory(rbs->device, vertex_buffer_memory, 0, vertex_buffer_mr.size, 0, (void**)&vertex_buffer_memory_data);
        VERIFY_RES();
    
        memcpy(vertex_buffer_memory_data, m->vertices, sizeof(MeshVertex) * m->vertices_num);
    
        vkUnmapMemory(rbs->device, vertex_buffer_memory);
    
        res = vkBindBufferMemory(rbs->device, vertex_buffer, vertex_buffer_memory, 0);
        VERIFY_RES();
    }


    // Indexbuffer

    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;
    {
        VkBufferCreateInfo index_bci = {};
        index_bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        index_bci.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        index_bci.size = sizeof(MeshIndex) * m->indices_num;
        index_bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
        res = vkCreateBuffer(rbs->device, &index_bci, NULL, &index_buffer);
        VERIFY_RES();

        VkMemoryRequirements index_buffer_mr;
        vkGetBufferMemoryRequirements(rbs->device, index_buffer, &index_buffer_mr);
        VkMemoryAllocateInfo index_buffer_mai = {};
        index_buffer_mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        index_buffer_mai.allocationSize = index_buffer_mr.size;
        index_buffer_mai.memoryTypeIndex = memory_type_from_properties(index_buffer_mr.memoryTypeBits, &rbs->gpu_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        check(index_buffer_mai.memoryTypeIndex != (u32)-1, "Couldn't find memory of correct type.");
    
        res = vkAllocateMemory(rbs->device, &index_buffer_mai, NULL, &index_buffer_memory);
        VERIFY_RES();
    
        u8* index_buffer_memory_data;
        res = vkMapMemory(rbs->device, index_buffer_memory, 0, index_buffer_mr.size, 0, (void**)&index_buffer_memory_data);
        VERIFY_RES();
    
        memcpy(index_buffer_memory_data, m->indices, sizeof(MeshIndex) * m->indices_num);
    
        vkUnmapMemory(rbs->device, index_buffer_memory);
    
        res = vkBindBufferMemory(rbs->device, index_buffer, index_buffer_memory, 0);
        VERIFY_RES();
    }

    RenderBackendMesh g = {
        .vertex_buffer = vertex_buffer,
        .vertex_buffer_memory = vertex_buffer_memory,
        .index_buffer = index_buffer,
        .index_buffer_memory = index_buffer_memory,
        .indices_num = m->indices_num
    };

    return mema_copy_t(&g, RenderBackendMesh);
}

void renderer_backend_update_constant_buffer(RendererBackend* rbs, C(RenderBackendPipeline) pipeline, u32 binding, const void* data, u32 data_size, u32 offset)
{
    u32 cf = rbs->current_frame;
    VkResult res;

    PipelineConstantBuffer* cb = NULL;
    u32 cb_idx = -1;

    for (u32 i = 0; i < pipeline.constant_buffers_num; ++i)
    {
        if (pipeline.constant_buffers[i].binding == binding)
        {
            cb = pipeline.constant_buffers + i;
            cb_idx = i;
            break;
        }
    }

    check(cb, "No constant buffer with binding %d in supplied pipeline", binding);

    u8* mapped_uniform_data;
    res = vkMapMemory(rbs->device, cb->memory[cf], offset, cb->allocated_size - offset, 0, (void**)&mapped_uniform_data);
    VERIFY_RES();
    memcpy(mapped_uniform_data, data, data_size);
    vkUnmapMemory(rbs->device, cb->memory[cf]);

    VkDescriptorBufferInfo dbi = {};
    dbi.buffer = cb->vk_handle[cf];
    dbi.range = cb->size;

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = pipeline.constant_buffer_descriptor_sets[cf][cb_idx];
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pBufferInfo = &dbi;

    vkUpdateDescriptorSets(rbs->device, 1, &write, 0, NULL);
}

static VkIndexType get_index_type(MeshIndex gi)
{
    switch(sizeof(gi))
    {
        case 2: return VK_INDEX_TYPE_UINT16;
        default: error("Implement me!");
    }
}

void renderer_backend_draw(RendererBackend* rbs, RenderBackendPipeline* pipeline, RenderBackendMesh* mesh)
{
    u32 cf = rbs->current_frame;
    VkResult res;
    VkCommandBufferBeginInfo cbbi = {};
    
    cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkCommandBuffer cmd = rbs->graphics_cmd_buffers[cf];
    res = vkBeginCommandBuffer(cmd, &cbbi);
    VERIFY_RES();

    SwapchainBuffer* scb = &rbs->swapchain_buffers[cf];

    VkClearValue clear_values[2];
    clear_values[0].color.float32[0] = 0.0f;
    clear_values[0].color.float32[1] = 0.0f;
    clear_values[0].color.float32[2] = 0.0f;
    clear_values[0].color.float32[3] = 1.0f;
    clear_values[1].depthStencil.depth = 1.0f;
    clear_values[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo rpbi = {};
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi.renderPass = rbs->render_pass;
    rpbi.framebuffer = scb->framebuffer;
    rpbi.renderArea.extent.width = rbs->swapchain_size.x;
    rpbi.renderArea.extent.height = rbs->swapchain_size.y;
    rpbi.clearValueCount = 2;

    rpbi.pClearValues = clear_values;
    vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_handle);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 0, pipeline->constant_buffers_num,
                            pipeline->constant_buffer_descriptor_sets[cf], 0, NULL);

    VkDeviceSize offsets[1] = {0};
    VkBuffer vertex_buffer = mesh->vertex_buffer;
    vkCmdBindVertexBuffers(cmd, 0, 1, &vertex_buffer, offsets);
    vkCmdBindIndexBuffer(cmd, mesh->index_buffer, 0, get_index_type(0));


    VkViewport viewport = {};
    viewport.width = rbs->swapchain_size.x;
    viewport.height = rbs->swapchain_size.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    viewport.x = 0;
    viewport.y = 0;
    vkCmdSetViewport(cmd, 0, 1, &viewport);


    VkRect2D scissor = {};
    scissor.extent.width = rbs->swapchain_size.x;
    scissor.extent.height = rbs->swapchain_size.y;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdDrawIndexed(cmd, mesh->indices_num, 1, 0, 0, 0);

    vkCmdEndRenderPass(cmd);

    res = vkEndCommandBuffer(cmd);
    VERIFY_RES();
}

void renderer_backend_present(RendererBackend* rbs)
{
    VkResult res;
    u32 cf = rbs->current_frame;

    u32 image_index;
    u32 timeout = 100000000; // 0.1 s
    res = vkAcquireNextImageKHR(rbs->device, rbs->swapchain, timeout, rbs->image_available_semaphores[cf], VK_NULL_HANDLE, &image_index);

    if (res == VK_ERROR_OUT_OF_DATE_KHR)
        return; // Couldn't present due to out of date swapchain, waiting for window resize to propagate.

    VERIFY_RES();

    VkPipelineStageFlags psf = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si = {}; // can be mupltiple!!
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.pWaitSemaphores = &rbs->image_available_semaphores[cf];
    si.waitSemaphoreCount = 1;
    si.pSignalSemaphores = &rbs->render_finished_semaphores[cf];
    si.signalSemaphoreCount = 1;
    si.pWaitDstStageMask = &psf;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &rbs->graphics_cmd_buffers[cf];

    vkResetFences(rbs->device, 1, &rbs->image_in_flight_fences[cf]);
    res = vkQueueSubmit(rbs->graphics_queue, 1, &si, rbs->image_in_flight_fences[cf]);
    VERIFY_RES();

    VkPresentInfoKHR pi = {};
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.swapchainCount = 1;
    pi.pSwapchains = &rbs->swapchain;
    pi.pImageIndices = &image_index;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &rbs->render_finished_semaphores[cf];

    res = vkQueuePresentKHR(rbs->present_queue, &pi);

    if (res == VK_ERROR_OUT_OF_DATE_KHR)
        return; // Couldn't present due to out of date swapchain, waiting for window resize to propagate.

    VERIFY_RES();

    rbs->current_frame = (rbs->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void renderer_backend_wait_for_new_frame(RendererBackend* rbs)
{
    vkWaitForFences(rbs->device, 1, &rbs->image_in_flight_fences[rbs->current_frame], VK_TRUE, UINT64_MAX);
}

void renderer_backend_wait_until_idle(RendererBackend* rbs)
{
    vkDeviceWaitIdle(rbs->device);
}

void renderer_backend_surface_resized(RendererBackend* rbs, u32 width, u32 height)
{
    (void)width;
    (void)height;
    info("Render backend resizing to %d x %d", width, height);
    recreate_surface_size_dependent_resources(rbs);
}

Vec2u renderer_backend_get_size(RendererBackend* rbs)
{
    return rbs->swapchain_size;
}