#pragma once

#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

#include "jsonxx.h"

#include "SDL_events.h"
#include "SDL_keysym.h"

namespace yadc {
    namespace input {
        bool initialize();
        static int (*push_event)(SDL::Event* event) = 0;
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
            bool read_from_json (std::string input);
            bool trigger();
            SDL::EventType type;
            SDL::Key sym;
            uint16_t unicode;
            uint16_t modstate;
        };
    }
}
