#include "renderer.h"
#include <assert.h>
#include <vulkan/vulkan.h>
#include "linux_xcb_window.h"
#include "memory.h"

typedef struct {
    VkImage image;
    VkImageView view;
} swapchain_buffer_t;

struct renderer_state_t {
};

renderer_state_t* renderer_init(window_type_e window_type, void* window_data)
{
    assert(window_type == WINDOW_TYPE_XCB);
    linux_xcb_window_t* win = window_data;
    (void)win;
    renderer_state_t* rs = zalloc_zero(sizeof(renderer_state_t));
    return rs;
}

void renderer_shutdown(renderer_state_t* rs)
{
    zfree(rs);
}