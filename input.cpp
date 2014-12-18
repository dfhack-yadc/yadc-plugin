#include "input.h"
#include "util.h"

#ifdef _DARWIN
#include <dlfcn.h>
#endif

#ifdef _LINUX
extern "C" {
    extern int SDL_PushEvent(SDL::Event* event);
}
#endif

using namespace yadc;
using namespace yadc::input;

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

bool KeyboardEvent::read_from_json (const jsonxx::Object object)
{
    using namespace jsonxx;
    // Allow {"keydown": true} or {"keydown": 1}
    bool is_keydown = (object.has<Boolean>("keydown"))
        ? object.get<Boolean>("keydown") : (bool)object.get<Number>("keydown", 0);
    type = (is_keydown) ? SDL::ET_KEYDOWN : SDL::ET_KEYUP;
    sym = (SDL::Key) object.get<Number>("sym", (int)SDL::K_UNKNOWN);
    unicode = (uint16_t) object.get<Number>("unicode", 0);
    modstate = (SDL::Mod)(
        (object.get<Boolean>("shift", false) ? SDL::KMOD_SHIFT : 0) |
        (object.get<Boolean>("alt", false)   ? SDL::KMOD_ALT   : 0) |
        (object.get<Boolean>("ctrl", false)  ? SDL::KMOD_CTRL  : 0)
    );
    return sym != SDL::K_UNKNOWN || unicode != 0;
}

bool KeyboardEvent::read_from_json (const std::string input)
{
    using namespace jsonxx;
    jsonxx::Object o;
    if (!o.parse(input))
        return false;
    return read_from_json(o);
}

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
    event.key.ksym.sym = sym;
    event.key.ksym.unicode = unicode;
    input::push_event(&event);
    return true;
}
