#pragma once

#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

#include "ActiveSocket.h"

using namespace DFHack;

namespace yadc {
    class Client {
    private:
        bool connected;
    protected:
        int16_t comm_port;
        int16_t screen_port;
        CActiveSocket* comm_socket;
        CActiveSocket* screen_socket;
        bool send_data (CActiveSocket* sock, const uint8_t* buffer, int32_t length);
        void handle_error (CSimpleSocket::CSocketError err);
        void cleanup();
    public:
        Client (int16_t comm_port, int16_t screen_port);
        ~Client();
        command_result connect();
        command_result disconnect();
        inline bool isConnected() { return connected; }
        bool send_screen_data (const uint8_t* buffer, int32_t length);
        bool send_comm_data (const uint8_t* buffer, int32_t length);
    };
}
