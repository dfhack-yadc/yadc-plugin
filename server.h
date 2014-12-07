#pragma once

#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

using namespace DFHack;

namespace yadc {
    class Server {
    protected:
        int16_t comm_port;
        int16_t screen_port;
    public:
        Server (int16_t comm_port, int16_t screen_port);
        ~Server();
        command_result start();
        command_result stop();
    };
}
