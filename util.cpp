#include "util.h"

using namespace DFHack;
using namespace yadc;

void util::print_color (color_ostream &out, color_value color,
                        const char * format, ...)
{
    va_list args;
    va_start(args, format);
    util::vprint_color(out, color, format, args);
    va_end(args);
}

void util::vprint_color (color_ostream &out, color_value color,
                         const char* format, va_list args)
{
    color_value save = out.color();
    out.color(color);
    out.vprint(format, args);
    out.color(save);
}
