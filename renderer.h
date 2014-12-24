#pragma once

#include "Core.h"
#include "ColorText.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

#include "tinythread.h"

#include "df/enabler.h"
#include "df/graphic.h"
#include "df/renderer.h"
#include "df/zoom_commands.h"

using df::global::gps;

namespace yadc {
    namespace renderer {
        enum renderer_event {
            GRID_RESIZE = 1
        };

        class YADCRenderer : public df::renderer {
        public:
            static const int DIRTY_LEN = 256 * 256;
            static const int OLD_BUFFER_LEN = 256 * 256 * 3;

            YADCRenderer (df::renderer* parent);
            ~YADCRenderer();
            virtual void update_tile (int32_t x, int32_t y);
            virtual void update_all();
            virtual void render();
            virtual void set_fullscreen();
            virtual void zoom (df::zoom_commands z);
            virtual void resize (int32_t w, int32_t h);
            virtual void grid_resize (int32_t w, int32_t h);
            virtual bool get_mouse_coords (int32_t* x, int32_t* y);
            virtual bool uses_opengl();

            int32_t serialize_changed (unsigned char* dest, int maxlength);
            int32_t serialize_events (unsigned char* dest, int maxlength);
        protected:
            df::renderer* parent;
        private:
            int event_flags;
            tthread::recursive_mutex * lock;
            unsigned char dirty[DIRTY_LEN];
            unsigned char old_buffer[OLD_BUFFER_LEN];
            void copy_from_inner();
            void copy_to_inner();
            inline void fill_dirty()  { memset(dirty, 1, DIRTY_LEN); };
            inline void clear_dirty() { memset(dirty, 0, DIRTY_LEN); };
            inline void reset_old_buffer() { memset(old_buffer, 255, OLD_BUFFER_LEN); };
        };

        void add_renderer (df::renderer *r);
        void remove_renderer();
        inline int tile_index (int x, int y) { return x * gps->dimy + y; }
    }
}
