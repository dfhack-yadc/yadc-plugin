#pragma once

#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

#include "jsoncpp.h"

#include "SDL_events.h"
#include "SDL_keysym.h"

namespace yadc {
    namespace input {
        static std::map<int, SDL::Key> sdl_key_map;

        bool init();
        SDL::Key map_key (int keycode);
        struct KeyboardEvent {
            KeyboardEvent():
                type(SDL::ET_NOEVENT),
                sym(SDL::K_UNKNOWN),
                unicode(0),
                modstate(0)
            { }
            KeyboardEvent(SDL::EventType type, SDL::Key sym, uint16_t unicode,
                            uint16_t modstate):
                type(type),
                sym(sym),
                unicode(unicode),
                modstate(modstate)
            { }
            bool read_from_json (const Json::Value &object);
            bool read_from_json (const std::string &input);
            bool trigger();
            SDL::EventType type;
            SDL::Key sym;
            uint16_t unicode;
            uint16_t modstate;
        };
    }
}
