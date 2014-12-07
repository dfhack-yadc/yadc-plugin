#pragma once

#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

using namespace DFHack;

namespace yadc {
    class Server {
        Server* instance;
    public:
        static command_result start();
        static command_result stop();
    };
}
