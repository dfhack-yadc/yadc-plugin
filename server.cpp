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

Server::~Server()
{
    if (connected)
        stop();
}

command_result Server::start()
{
    if (connected)
        return CR_FAILURE;
    comm_socket = new CActiveSocket;
    comm_socket->Initialize();
    comm_socket->SetNonblocking();
    if (!comm_socket->Open((uint8_t*)"127.0.0.1", comm_port))
    {
        util::log("Failed to connect to 127.0.0.1:%i \n", comm_port);
        delete comm_socket;
        return CR_FAILURE;
    }

    screen_socket = new CActiveSocket;
    screen_socket->Initialize();
    screen_socket->SetNonblocking();
    if (!screen_socket->Open((uint8_t*)"127.0.0.1", screen_port))
    {
        util::log("Failed to connect to 127.0.0.1:%i\n", screen_port);
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
        if (comm_socket)
        {
            comm_socket->Close();
            delete comm_socket;
            comm_socket = NULL;
        }
        if (screen_socket)
        {
            screen_socket->Close();
            delete screen_socket;
            screen_socket = NULL;
        }
    }
    return CR_OK;
}

bool Server::sendScreenData (const unsigned char* buffer, int length)
{
    if (!screen_socket)
        return false;
    int result = screen_socket->Send((const uint8_t*)buffer, length);
    return (result != -1);
}
