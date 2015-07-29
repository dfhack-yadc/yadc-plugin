#include "screen.h"
#include "util.h"

#include "df/enabler.h"
#include "df/graphic.h"
#include "df/renderer.h"

using df::global::enabler;
using df::global::gps;

using namespace DFHack;
using namespace tthread;
using namespace yadc;

static uint8_t old_buffer[256 * 256 * 3];
static struct {
    uint8_t tiles;
    uint8_t events;
} update;
static float colors[16][3];

void screen::invalidate()
{
    memset(&update, 1, sizeof(update));
}

uint32_t screen::serialize_changed (uint8_t* dest, int maxlength)
{
    // Serialize all changed tiles
    // Each tile is represented by 5 bytes: x, y, ch, fg, bg
    static int dimx = -1, dimy = -1;
    uint8_t* p = dest;
    for (int y = 0; y < gps->dimy; y++)
    {
        for (int x = 0; x < gps->dimx; x++)
        {
            if ((int)(p + 5) - (int)dest > maxlength)
                break;
            const int raw_tile = x * gps->dimy + y;
            // old_buffer is a constant size, which ensures that tiles are
            // properly updated when the screen dimensions change
            uint8_t* old_buffer_tile = old_buffer + ((x * 256 + y) * 3);
            uint8_t* screen_tile = (uint8_t*)enabler->renderer->screen + (raw_tile * 4);
            uint8_t ch = screen_tile[0];
            uint8_t fg = (screen_tile[1] + (8 * screen_tile[3])) % 16;
            uint8_t bg = screen_tile[2] % 16;
            if (ch == 0 || ch == 32)
            {
                // characters 0 and 32 should be identical in tilesets, due to
                // both being used for blank space in various places
                // additionally, their foreground color does not matter
                ch = 0;
                fg = 0;
            }
            if (!update.tiles &&
                old_buffer_tile[0] == ch &&
                old_buffer_tile[1] == fg &&
                old_buffer_tile[2] == bg &&
                // Send newly-visible tiles, even if the same tiles have been displayed
                // in this location previously
                x < dimx && y < dimy
            )
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
    dimx = gps->dimx;
    dimy = gps->dimy;
    update.tiles = false;
    uint32_t len = (uint32_t)(p - dest);
    return len;
}

bool update_colors()
{
    bool changed = false;
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (colors[i][j] != enabler->ccolor[i][j])
            {
                colors[i][j] = enabler->ccolor[i][j];
                changed = true;
            }
        }
    }
    return changed;
}

uint32_t screen::serialize_events (uint8_t* dest, int maxlength)
{
    static int dimx = -1, dimy = -1;
    Json::Value events;
    if (update.events || dimx != gps->dimx || dimy != gps->dimy)
    {
        events["dims"]["x"] = gps->dimx;
        events["dims"]["y"] = gps->dimy;
        dimx = gps->dimx;
        dimy = gps->dimy;
    }
    bool colors_changed = update_colors();
    if (update.events || colors_changed)
    {
        Json::Value all_colors;
        for (int i = 0; i < 16; i++)
        {
            Json::Value cur_color;
            for (int j = 0; j < 3; j++)
                cur_color.append((int)(255.0 * colors[i][j]));
            all_colors.append(cur_color);
        }
        events["colors"] = all_colors;
    }
    if (events.empty())
        return 0;
    std::string json = JsonEx::toSimpleString(events);
    strncpy((char*)dest, json.c_str(), maxlength);
    update.events = false;
    return json.size();
}
