#pragma once

#include "Core.h"
#include "ColorText.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

#include "df/enabler.h"
#include "df/graphic.h"
#include "df/renderer.h"
#include "df/zoom_commands.h"

namespace yadc {
    namespace renderer {
        struct old_opengl : public df::renderer
        {
            void* sdlSurface;
            int32_t dispx, dispy;
            float *vertexes, *fg, *bg, *tex;
            int32_t zoom_steps, forced_steps, natural_w, natural_h;
            int32_t off_x, off_y, size_x, size_y;
        };

        class YADCRenderer : public df::renderer {
        public:
            static const int DIRTY_LEN = 256 * 256;

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
        protected:
            df::renderer* parent;
        private:
            unsigned char dirty[DIRTY_LEN];
            void copy_from_inner();
            void copy_to_inner();
            void invalidate_all();
        };

        void add_renderer (df::renderer *r);
        void remove_renderer();
    }
}
