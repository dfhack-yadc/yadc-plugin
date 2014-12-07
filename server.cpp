#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

#include "server.h"

using namespace yadc;

Server::Server(int16_t comm_port, int16_t screen_port)
    :comm_port(comm_port),
     screen_port(screen_port)
{ }

Server::~Server() { }

command_result Server::start()
{
    return CR_OK;
}

command_result Server::stop()
{
    return CR_OK;
}
