#include "util.h"

using namespace DFHack;
using namespace yadc;

bool util::init_log_file()
{
    if (log_file)
        return true;
    log_file = fopen("yadc.log", "w");
    if (log_file == NULL)
    {
        log_file = stderr;
        return false;
    }
    return true;
}

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

void util::log (const char* format, ...)
{
    va_list args;
    va_start(args, format);
    util::vlog(format, args);
    va_end(args);
}

void util::vlog (const char* format, va_list args)
{
    init_log_file();
    vfprintf(log_file, format, args);
    fflush(log_file);
}
