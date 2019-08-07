#include "renderer.h"
#include <assert.h>
#include <vulkan/vulkan.h>
#include "linux_xcb_window.h"

typedef struct {
    VkImage image;
    VkImageView view;
} swapchain_buffer_t;

void renderer_init(window_type_e window_type, void* window_data)
{
    assert(window_type == WINDOW_TYPE_XCB);
    linux_xcb_window_t* win = window_data;
    (void)win;
}