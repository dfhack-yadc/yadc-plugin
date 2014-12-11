#pragma once

#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

#include "ActiveSocket.h"

using namespace DFHack;

namespace yadc {
    class Server {
        bool connected;
    protected:
        int16_t comm_port;
        int16_t screen_port;
        CActiveSocket* comm_socket;
        CActiveSocket* screen_socket;
        void log(std::string msg, bool console);
    public:
        Server (int16_t comm_port, int16_t screen_port);
        ~Server();
        command_result start();
        command_result stop();
        bool sendScreenData (const unsigned char* buffer, int length);
    };
}
