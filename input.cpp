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
#define MAP(a, b) sdl_key_map.insert(std::pair<int, SDL::Key>(a, b))
    // Modifiers
    MAP(16, SDL::K_LSHIFT);
    MAP(17, SDL::K_LCTRL);
    MAP(18, SDL::K_LALT);

    MAP(27, SDL::K_ESCAPE);

    MAP(33, SDL::K_PAGEUP);
    MAP(34, SDL::K_PAGEDOWN);
    MAP(35, SDL::K_END);
    MAP(36, SDL::K_HOME);
    MAP(37, SDL::K_LEFT);
    MAP(39, SDL::K_RIGHT);
    MAP(38, SDL::K_UP);
    MAP(40, SDL::K_DOWN);

    MAP(45, SDL::K_INSERT);
    MAP(46, SDL::K_DELETE);

    // 48 - 57 map to SDL_K0 - SDL_K9

    // Map uppercase characters to lowercase SDL key codes
    MAP(65, SDL::K_a);
    MAP(66, SDL::K_b);
    MAP(67, SDL::K_c);
    MAP(68, SDL::K_d);
    MAP(69, SDL::K_e);
    MAP(70, SDL::K_f);
    MAP(71, SDL::K_g);
    MAP(72, SDL::K_h);
    MAP(73, SDL::K_i);
    MAP(74, SDL::K_j);
    MAP(75, SDL::K_k);
    MAP(76, SDL::K_l);
    MAP(77, SDL::K_m);
    MAP(78, SDL::K_n);
    MAP(79, SDL::K_o);
    MAP(80, SDL::K_p);
    MAP(81, SDL::K_q);
    MAP(82, SDL::K_r);
    MAP(83, SDL::K_s);
    MAP(84, SDL::K_t);
    MAP(85, SDL::K_u);
    MAP(86, SDL::K_v);
    MAP(87, SDL::K_w);
    MAP(88, SDL::K_x);
    MAP(89, SDL::K_y);
    MAP(90, SDL::K_z);

    // These are ignored by DF, but would be treated as K_LEFTBRACKET and
    // K_BACKSLASH (respectively) otherwise.
    MAP(91, SDL::K_LMETA);
    MAP(92, SDL::K_RMETA);

    // Numpad keys
    MAP(96, SDL::K_KP0);
    MAP(97, SDL::K_KP1);
    MAP(98, SDL::K_KP2);
    MAP(99, SDL::K_KP3);
    MAP(100, SDL::K_KP4);
    MAP(101, SDL::K_KP5);
    MAP(102, SDL::K_KP6);
    MAP(103, SDL::K_KP7);
    MAP(104, SDL::K_KP8);
    MAP(105, SDL::K_KP9);
    MAP(106, SDL::K_KP_MULTIPLY);
    MAP(107, SDL::K_KP_PLUS);
    MAP(109, SDL::K_KP_MINUS);
    MAP(110, SDL::K_KP_PERIOD);
    MAP(111, SDL::K_KP_DIVIDE);

    // F* keys
    MAP(112, SDL::K_F1);
    MAP(113, SDL::K_F2);
    MAP(114, SDL::K_F3);
    MAP(115, SDL::K_F4);
    MAP(116, SDL::K_F5);
    MAP(117, SDL::K_F6);
    MAP(118, SDL::K_F7);
    MAP(119, SDL::K_F8);
    MAP(120, SDL::K_F9);
    MAP(121, SDL::K_F10);
    MAP(122, SDL::K_F11);
    MAP(123, SDL::K_F12);

    // Punctuation
    MAP(186, SDL::K_SEMICOLON);
    MAP(187, SDL::K_EQUALS);
    MAP(188, SDL::K_COMMA);
    MAP(189, SDL::K_MINUS);
    MAP(190, SDL::K_PERIOD);
    MAP(191, SDL::K_SLASH);
    MAP(192, SDL::K_BACKQUOTE);

    MAP(219, SDL::K_LEFTBRACKET);
    MAP(220, SDL::K_BACKSLASH);
    MAP(221, SDL::K_RIGHTBRACKET);
    MAP(222, SDL::K_QUOTE);

#undef MAP
    return true;
}

SDL::Key input::map_key (int keycode)
{
    auto item = sdl_key_map.find(keycode);
    if (item != sdl_key_map.end())
        return item->second;
    return (SDL::Key)keycode;
}

bool KeyboardEvent::read_from_json (const jsonxx::Object object)
{
    using namespace jsonxx;
    // Allow {"keydown": true} or {"keydown": 1}
    bool is_keydown = (object.has<Boolean>("keydown"))
        ? object.get<Boolean>("keydown") : (bool)object.get<Number>("keydown", 0);
    type = (is_keydown) ? SDL::ET_KEYDOWN : SDL::ET_KEYUP;
    sym = map_key(object.get<Number>("sym", (int)SDL::K_UNKNOWN));
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
    if (!(type == SDL::ET_KEYDOWN || type == SDL::ET_KEYUP))
        return false;
    SDL::Event event;
    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key.state = (type == SDL::ET_KEYDOWN) ? SDL::BTN_PRESSED : SDL::BTN_RELEASED;
    event.key.ksym.mod = (SDL::Mod)modstate;
    event.key.ksym.sym = sym;
    event.key.ksym.unicode = unicode;
    SDL_PushEvent(&event);
    return true;
}
