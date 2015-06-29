#pragma once

#include "Core.h"
#include "ColorText.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

#include "tinythread.h"

namespace yadc {
    namespace screen {
        void invalidate();
        uint32_t serialize_changed (uint8_t* dest, int maxlength);
        uint32_t serialize_events (uint8_t* dest, int maxlength);
    }
}
