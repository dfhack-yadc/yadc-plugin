#include "input.h"
#include "util.h"

using namespace yadc;
using namespace yadc::input;

#ifdef _DARWIN
#include <dlfcn.h>
#endif

bool input::initialize()
{
#if defined(_LINUX)
    input::push_event = SDL_PushEvent;
#elif defined(_DARWIN)
    void* sdl_library = dlopen("SDL.framework/SDL", RTLD_LAZY);
    if (sdl_library)
    {
        input::push_event = (int (*)(SDL::Event*))
            dlsym(sdl_library, "SDL_PushEvent");
        dlclose(sdl_library);
    }
#else
    input::push_event = (int (*)(SDL::Event*))
        GetProcAddress(GetModuleHandle("SDLreal.dll"), "SDL_PushEvent");
#endif
    util::log("push_event: 0x%x\n", input::push_event);
    return input::push_event != 0;
}

bool KeyboardEvent::read_from_json (std::string input) {}

bool KeyboardEvent::trigger()
{
    if (!input::push_event)
        return false;
    if (!(type == SDL::ET_KEYDOWN || type == SDL::ET_KEYUP))
        return false;
    SDL::Event event;
    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key.state = (type == SDL::ET_KEYDOWN) ? SDL::BTN_PRESSED : SDL::BTN_RELEASED;
    event.key.ksym.mod = (SDL::Mod)modstate;
    event.key.ksym.sym = SDL::K_a;
    event.key.ksym.unicode = 97;
    input::push_event(&event);
    return true;
}
