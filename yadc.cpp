#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"
#include "modules/Filesystem.h"

#include "jsonxx.h"

#include "input.h"
#include "client.h"
#include "config.h"
#include "renderer.h"
#include "util.h"

using namespace DFHack;
using namespace df::enums;
using namespace yadc;

DFHACK_PLUGIN("yadc");
DFHACK_PLUGIN_IS_ENABLED(is_enabled);

using df::global::enabler;
using df::global::gps;

static Client* client;
static config::YADCConfig yadc_config;

DFhackCExport command_result plugin_init (color_ostream &out, std::vector <PluginCommand> &commands);
DFhackCExport command_result plugin_shutdown (color_ostream &out);
DFhackCExport command_result plugin_onupdate (color_ostream &out);
DFhackCExport command_result plugin_enable (color_ostream &out, bool enable);
command_result cmd_yadc(color_ostream &out, std::vector <std::string> & parameters);

command_result client_connect()
{
    if (client)
        return CR_FAILURE;
    client = new Client(yadc_config.comm_port, yadc_config.screen_port);
    command_result res = client->connect();
    if (res != CR_OK)
    {
        delete client;
        client = NULL;
    }
    return res;
}

command_result client_disconnect()
{
    if (!client)
        return CR_FAILURE;
    client->disconnect();
    delete client;
    client = NULL;
    return CR_OK;
}

DFhackCExport command_result plugin_init (color_ostream &out, std::vector <PluginCommand> &commands)
{
    if (!input::initialize())
    {
        out.printerr("Failed to initialize input hooks\n");
        return CR_FAILURE;
    }
    DFHack::Filesystem::mkdir("yadc");
    util::init_log_file();
    if (!config::init_config())
    {
        out.printerr("yadc: Could not initialize configuration file\n");
        return CR_FAILURE;
    }
    config::ConfigParser parser(YADC_CONFIG_PATH);
    if (!parser.isValid())
    {
        out.printerr("yadc: Could not load configuration file\n");
        return CR_FAILURE;
    }
    jsonxx::Object config = parser.getData();
    yadc_config.comm_port = config.get<jsonxx::Number>("comm_port", 25143);
    yadc_config.screen_port = config.get<jsonxx::Number>("screen_port", 25144);
    commands.push_back(PluginCommand(
        "yadc", "yadc",
        cmd_yadc, false,
        "  yadc [status]: Display client status\n"
        "  yadc connect|start: Connect to the server\n"
        "  yadc disconnect|stop: Disconnect from the server\n"
        "  yadc reconnect|restart: Restart the client\n"
    ));
    if (getenv("YADC_AUTO_ENABLE"))
    {
        command_result res = plugin_enable(out, true);
        if (res == CR_OK)
            util::print_color(out, COLOR_LIGHTGREEN, "Auto-enabled yadc\n");
        else
            util::print_color(out, COLOR_LIGHTMAGENTA, "Could not auto-enable yadc\n");
    }
    return CR_OK;
}

DFhackCExport command_result plugin_shutdown (color_ostream &out)
{
    if (client)
        return client_disconnect();
    if (is_enabled)
    {
        // Clean up
        plugin_enable(out, false);
    }
    return CR_OK;
}

unsigned char test_buffer[256 * 256 * 5];
DFhackCExport command_result plugin_onupdate (color_ostream &out)
{
    static int32_t last_gpu_tick = 0;
    int32_t len;
    if (is_enabled && enabler->gputicks.value != last_gpu_tick)
    {
        last_gpu_tick = enabler->gputicks.value;
        CoreSuspender suspend;
        renderer::YADCRenderer* r = static_cast<renderer::YADCRenderer*>(enabler->renderer);
        len = r->serialize_changed(test_buffer, 256 * 256 * 5);
        if (len)
        {
            client->send_screen_data(test_buffer, len);
        }
        len = r->serialize_events(test_buffer, 256 * 256 * 5);
        if (len)
        {
            client->send_comm_data(test_buffer, len);
        }
    }
    return CR_OK;
}

DFhackCExport command_result plugin_enable (color_ostream &out, bool enable)
{
    if (enable != is_enabled)
    {
        command_result res;
        if (is_enabled)
            res = client_disconnect();
        else
            res = client_connect();
        if (res != CR_OK)
        {
            out.printerr("Could not %s client.\n", (enable) ? "start" : "stop");
            return res;
        }
        if (enable)
        {
            renderer::add_renderer(new renderer::YADCRenderer(enabler->renderer));
        }
        else
        {
            renderer::remove_renderer();
        }
        is_enabled = enable;
        util::log("Plugin %s.\n", (is_enabled) ? "enabled" : "disabled");
        return res;
    }
    else {
        util::print_color(out, (is_enabled) ? COLOR_LIGHTGREEN : COLOR_YELLOW,
            "Client is already %s\n", (is_enabled) ? "connected" : "disconnected");
        return CR_FAILURE;
    }
}

command_result cmd_yadc_status(color_ostream &out)
{
    out.print("Client %s\n", (is_enabled) ? "connected" : "disconnected");
    return CR_OK;
}

command_result cmd_yadc(color_ostream &out, std::vector <std::string> &parameters)
{
    CoreSuspender suspend;
    if (parameters.size() >= 1)
    {
        if (parameters[0] == "start" || parameters[0] == "enable" || parameters[0] == "connect")
        {
            return plugin_enable(out, true);
        }
        else if (parameters[0] == "stop" || parameters[0] == "disable" || parameters[0] == "disconnect")
        {
            return plugin_enable(out, false);
        }
        else if (parameters[0] == "restart" || parameters[0] == "reconnect")
        {
            command_result res = CR_OK;
            if (is_enabled)
                res = plugin_enable(out, false);
            if (res == CR_OK)
                res = plugin_enable(out, true);
            return res;
        }
        else if (parameters[0] == "status")
            return cmd_yadc_status(out);
    }
    else if (parameters.size() == 0)
        return cmd_yadc_status(out);
    return CR_WRONG_USAGE;
}
