#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"
#include "DFHackVersion.h"

#include "client.h"
#include "config.h"
#include "util.h"

using namespace yadc;

Client::Client(int16_t comm_port, int16_t screen_port)
    :connected(false),
     comm_port(comm_port),
     screen_port(screen_port),
     comm_socket(NULL),
     screen_socket(NULL)
{ }

Client::~Client()
{
    if (connected)
        disconnect();
}

command_result Client::connect()
{
    if (connected)
        return CR_FAILURE;
    comm_socket = new CActiveSocket;
    comm_socket->Initialize();
    comm_socket->SetNonblocking();
    if (!comm_socket->Open("127.0.0.1", comm_port))
    {
        util::log("Failed to connect to 127.0.0.1:%i \n", comm_port);
        cleanup();
        return CR_FAILURE;
    }

    screen_socket = new CActiveSocket;
    screen_socket->Initialize();
    screen_socket->SetNonblocking();
    if (!screen_socket->Open("127.0.0.1", screen_port))
    {
        util::log("Failed to connect to 127.0.0.1:%i\n", screen_port);
        cleanup();
        return CR_FAILURE;
    }

    std::string df_id = util::unique_id();
    util::log("DF identifier: %s\n", df_id.c_str());
    if (comm_socket->Send((uint8_t*)df_id.c_str(), 8) != 8)
    {
        util::log("Handshake failed: 127.0.0.1:%i\n", comm_port);
        cleanup();
        return CR_FAILURE;
    }
    if (screen_socket->Send((uint8_t*)df_id.c_str(), 8) != 8)
    {
        util::log("Handshake failed: 127.0.0.1:%i\n", screen_port);
        cleanup();
        return CR_FAILURE;
    }

    connected = true;

    Json::Value info;
    info["info"]["df_version"] = std::string(Version::df_version());
    info["info"]["dfhack_version"] = std::string(Version::dfhack_version());
    info["info"]["name"] = config::get_config().name;
    std::string info_json = Json::toSimpleString(info);
    util::log("%s\n", info_json.c_str());
    if (!send_comm_data((uint8_t*)info_json.c_str(), info_json.size()))
    {
        util::log("Could not send game information\n");
        disconnect();
        return CR_FAILURE;
    }

    return CR_OK;
}

command_result Client::disconnect()
{
    if (connected)
        cleanup();
    connected = false;
    return CR_OK;
}

void Client::cleanup()
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

bool Client::send_data (CActiveSocket* sock, const uint8_t* buffer, int32_t length)
{
    if (!sock)
        return false;
    std::string str_length = util::int32_to_str(length);
    if (sock->Send((uint8_t*)str_length.c_str(), 4) != -1 &&
        sock->Send(buffer, length) != -1)
        return true;
    handle_error(sock->GetSocketError());
    return false;
}

void Client::handle_error (CSimpleSocket::CSocketError err)
{
    switch (err)
    {
        case CSimpleSocket::SocketConnectionRefused:
        case CSimpleSocket::SocketNotconnected:
        case CSimpleSocket::SocketInterrupted:
        case CSimpleSocket::SocketConnectionAborted:
        case CSimpleSocket::SocketProtocolError:
        case CSimpleSocket::SocketConnectionReset:
        case CSimpleSocket::SocketInvalidSocket:
            util::log("Fatal socket error: %i: %s\n", err, CSimpleSocket::DescribeError(err));
            disconnect();
            break;
        default:
            util::log("Socket error: %i: %s\n", err, CSimpleSocket::DescribeError(err));
            break;
    }
}

bool Client::send_screen_data (const uint8_t* buffer, int32_t length)
{
    return send_data(screen_socket, buffer, length);
}

bool Client::send_comm_data (const uint8_t* buffer, int32_t length)
{
    return send_data(comm_socket, buffer, length);
}
