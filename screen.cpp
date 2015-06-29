#include "screen.h"
#include "util.h"
#include "jsonxx.h"

#include "df/enabler.h"
#include "df/graphic.h"
#include "df/renderer.h"

using df::global::enabler;
using df::global::gps;

using namespace DFHack;
using namespace tthread;
using namespace yadc;

static uint8_t old_buffer[256 * 256 * 3];
static bool force_update;

void screen::invalidate() { force_update = true; }

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
            if (!force_update &&
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
    force_update = false;
    uint32_t len = (uint32_t)(p - dest);
    return len;
}

uint32_t screen::serialize_events (uint8_t* dest, int maxlength)
{
    static int dimx = -1, dimy = -1;
    jsonxx::Object events;
    if (dimx != gps->dimx || dimy != gps->dimy)
    {
        jsonxx::Object dims;
        dims << "x" << gps->dimx;
        dims << "y" << gps->dimy;
        events << "grid" << dims;
        dimx = gps->dimx;
        dimy = gps->dimy;
    }
    if (events.empty())
        return 0;
    std::string json = events.json();
    strncpy((char*)dest, json.c_str(), maxlength);
    return json.size();
}
