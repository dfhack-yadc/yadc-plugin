#include "renderer.h"
#include "util.h"

using df::global::enabler;
using df::global::gps;

using namespace DFHack;
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
    copy_from_inner();
}

YADCRenderer::~YADCRenderer()
{
    enabler->renderer = parent;
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

void YADCRenderer::invalidate_all()
{
    memset(dirty, 1, DIRTY_LEN);
}

void YADCRenderer::update_tile (int32_t x, int32_t y)
{
    copy_to_inner();
    parent->update_tile(x, y);
}

void YADCRenderer::update_all()
{
    copy_to_inner();
    parent->update_all();
}

void YADCRenderer::render()
{
    copy_to_inner();
    parent->render();
}

void YADCRenderer::set_fullscreen()
{
    copy_to_inner();
    parent->set_fullscreen();
    copy_from_inner();
}

void YADCRenderer::zoom (df::zoom_commands z)
{
    copy_to_inner();
    parent->zoom(z);
    copy_from_inner();
}

void YADCRenderer::resize (int32_t w, int32_t h)
{
    copy_to_inner();
    parent->resize(w, h);
    copy_from_inner();
}

void YADCRenderer::grid_resize (int32_t w, int32_t h)
{
    copy_to_inner();
    parent->grid_resize(w, h);
    copy_from_inner();
};

bool YADCRenderer::get_mouse_coords (int32_t* x, int32_t* y)
{
    return parent->get_mouse_coords(x, y);
}

bool YADCRenderer::uses_opengl()
{
    return parent->uses_opengl();
}

