#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

#include "server.h"
#include "util.h"

using namespace yadc;

Server::Server(int16_t comm_port, int16_t screen_port)
    :connected(false),
     comm_port(comm_port),
     screen_port(screen_port)
{ }

Server::~Server() { }

command_result Server::start()
{
    if (connected)
        return CR_FAILURE;
    comm_socket = new CActiveSocket;
    comm_socket->Initialize();
    comm_socket->SetNonblocking();
    screen_socket = new CActiveSocket;
    screen_socket->Initialize();
    screen_socket->SetNonblocking();
    if (!comm_socket->Open((uint8_t*)"127.0.0.1", comm_port) ||
        !screen_socket->Open((uint8_t*)"127.0.0.1", screen_port))
    {
        util::log_error("Failed to connect to 127.0.0.1:%i or 127.0.0.1:%i\n",
                comm_port, screen_port);
        delete comm_socket;
        delete screen_socket;
        return CR_FAILURE;
    }
    connected = true;
    return CR_OK;
}

command_result Server::stop()
{
    if (connected)
    {
        comm_socket->Close();
        delete comm_socket;
        screen_socket->Close();
        delete screen_socket;
    }
    return CR_OK;
}
