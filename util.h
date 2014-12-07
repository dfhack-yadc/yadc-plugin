#pragma once

#include <stdarg.h>

#include "Core.h"
#include "ColorText.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

using namespace DFHack;

namespace yadc {
    namespace util {
        void print_color (color_ostream &out, color_value color,
                          const char* format, ...);
        void vprint_color (color_ostream &out, color_value color,
                           const char* format, va_list args);
    }
}
