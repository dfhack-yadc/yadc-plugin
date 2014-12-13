#include "renderer.h"
#include "util.h"

using df::global::enabler;
using df::global::gps;

using namespace DFHack;
using namespace tthread;
using namespace yadc;
using namespace yadc::renderer;

void renderer::add_renderer (df::renderer *r)
{
    CoreSuspender suspend;
    enabler->renderer = r;
}

void renderer::remove_renderer()
{
    CoreSuspender suspend;
    delete enabler->renderer;
}

YADCRenderer::YADCRenderer (df::renderer* parent)
    :parent(parent)
{
    lock = new recursive_mutex();
    copy_from_inner();
    fill_dirty();
    reset_old_buffer();
}

YADCRenderer::~YADCRenderer()
{
    enabler->renderer = parent;
    delete[] dirty;
    delete[] old_buffer;
}

void YADCRenderer::copy_from_inner()
{
    screen = parent->screen;
    screentexpos = parent->screentexpos;
    screentexpos_addcolor = parent->screentexpos_addcolor;
    screentexpos_grayscale = parent->screentexpos_grayscale;
    screentexpos_cf = parent->screentexpos_cf;
    screentexpos_cbr = parent->screentexpos_cbr;
    screen_old = parent->screen_old;
    screentexpos_old = parent->screentexpos_old;
    screentexpos_addcolor_old = parent->screentexpos_addcolor_old;
    screentexpos_grayscale_old = parent->screentexpos_grayscale_old;
    screentexpos_cf_old = parent->screentexpos_cf_old;
    screentexpos_cbr_old = parent->screentexpos_cbr_old;
}

void YADCRenderer::copy_to_inner()
{
    parent->screen = screen;
    parent->screentexpos = screentexpos;
    parent->screentexpos_addcolor = screentexpos_addcolor;
    parent->screentexpos_grayscale = screentexpos_grayscale;
    parent->screentexpos_cf = screentexpos_cf;
    parent->screentexpos_cbr = screentexpos_cbr;
    parent->screen_old = screen_old;
    parent->screentexpos_old = screentexpos_old;
    parent->screentexpos_addcolor_old = screentexpos_addcolor_old;
    parent->screentexpos_grayscale_old = screentexpos_grayscale_old;
    parent->screentexpos_cf_old = screentexpos_cf_old;
    parent->screentexpos_cbr_old = screentexpos_cbr_old;
}

void YADCRenderer::update_tile (int32_t x, int32_t y)
{
    lock_guard <recursive_mutex> g(*lock);
    copy_to_inner();
    parent->update_tile(x, y);
    dirty[tile_index(x, y)] = 1;
}

void YADCRenderer::update_all()
{
    lock_guard <recursive_mutex> g(*lock);
    copy_to_inner();
    parent->update_all();
    copy_from_inner();
    fill_dirty();
}

void YADCRenderer::render()
{
    lock_guard <recursive_mutex> g(*lock);
    copy_to_inner();
    parent->render();
    copy_from_inner();
}

void YADCRenderer::set_fullscreen()
{
    lock_guard <recursive_mutex> g(*lock);
    copy_to_inner();
    parent->set_fullscreen();
    copy_from_inner();
    fill_dirty();
}

void YADCRenderer::zoom (df::zoom_commands z)
{
    lock_guard <recursive_mutex> g(*lock);
    copy_to_inner();
    parent->zoom(z);
    copy_from_inner();
    fill_dirty();
}

void YADCRenderer::resize (int32_t w, int32_t h)
{
    lock_guard <recursive_mutex> g(*lock);
    copy_to_inner();
    parent->resize(w, h);
    copy_from_inner();
    fill_dirty();
}

void YADCRenderer::grid_resize (int32_t w, int32_t h)
{
    lock_guard <recursive_mutex> g(*lock);
    copy_to_inner();
    parent->grid_resize(w, h);
    copy_from_inner();
    fill_dirty();
};

bool YADCRenderer::get_mouse_coords (int32_t* x, int32_t* y)
{
    return parent->get_mouse_coords(x, y);
}

bool YADCRenderer::uses_opengl()
{
    return parent->uses_opengl();
}

int32_t YADCRenderer::serialize_changed (unsigned char* dest, int maxlength)
{
    // Serialize all changed tiles
    // Each tile is represented by 5 bytes: x, y, tile, fg, bg
    lock_guard <recursive_mutex> g(*lock);
    unsigned char* p = dest;
    for (int y = 0; y < gps->dimy; y++)
    {
        for (int x = 0; x < gps->dimx; x++)
        {
            if ((int)(p + 5) - (int)dest > maxlength)
                break;
            const int tile = tile_index(x, y);
            if (!dirty[tile])
                continue;
            // old_buffer is a constant size, which ensures that tiles are
            // properly updated when the screen dimensions change
            unsigned char* old_buffer_tile = old_buffer + ((x * 256 + y) * 3);
            unsigned char* screen_tile = screen + (tile * 4);
            unsigned char ch = screen_tile[0];
            unsigned char fg = (screen_tile[1] + (8 * screen_tile[3])) % 16;
            unsigned char bg = screen_tile[2] % 8;
            // Avoid sending tiles that haven't actually changed, even if they
            // were updated (for example, after resizing the screen)
            if (old_buffer_tile[0] == ch && old_buffer_tile[1] == fg &&
                old_buffer_tile[2] == bg)
                continue;
            *(p++) = x;
            *(p++) = y;
            *(p++) = ch;
            *(p++) = fg;
            *(p++) = bg;
            old_buffer_tile[0] = ch;
            old_buffer_tile[1] = fg;
            old_buffer_tile[2] = bg;
        }
    }
    int32_t len = (int)(p - dest);
    clear_dirty();
    return len;
}

