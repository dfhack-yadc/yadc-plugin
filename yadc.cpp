#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"
#include "DFHackVersion.h"
#include "modules/Filesystem.h"

#include "df/enabler.h"

#include "input.h"
#include "client.h"
#include "config.h"
#include "screen.h"
#include "util.h"

using namespace DFHack;
using namespace df::enums;
using namespace yadc;

DFHACK_PLUGIN("yadc");
DFHACK_PLUGIN_IS_ENABLED(is_enabled);

REQUIRE_GLOBAL(enabler);
REQUIRE_GLOBAL(gps);

static Client* client;
static config::YADCConfig yadc_config;
config::YADCConfig &config::get_config() { return yadc_config; }

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
    if (res == CR_OK)
        yadc::screen::invalidate();
    else
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

bool load_config (color_ostream &out)
{
    config::ConfigParser parser(YADC_CONFIG_PATH);
    if (!parser.valid)
    {
        out.printerr("yadc: Could not load configuration file\n");
        return false;
    }
    yadc_config.comm_port = Json::get<int>(parser.data, "comm_port", 25143);
    yadc_config.screen_port = Json::get<int>(parser.data, "screen_port", 25144);
    std::string default_name = std::string("Unnamed DF ") + Version::df_version() + " game";
    yadc_config.name = Json::get<std::string>(parser.data, "name", default_name);
    return true;
}

DFhackCExport command_result plugin_init (color_ostream &out, std::vector <PluginCommand> &commands)
{
    if (!input::init())
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
    if (!load_config(out))
        return CR_FAILURE;
    commands.push_back(PluginCommand(
        "yadc", "yadc",
        cmd_yadc, false,
        "  yadc [status]: Display client status\n"
        "  yadc connect|start: Connect to the server\n"
        "  yadc disconnect|stop: Disconnect from the server\n"
        "  yadc reconnect|restart: Restart the client\n"
        "  yadc reload|reload-config: Reload the configuration file (" YADC_CONFIG_PATH ")\n"
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
    util::log("Plugin shutting down\n");
    if (is_enabled)
    {
        if (plugin_enable(out, false) != CR_OK)
            return CR_FAILURE;
    }
    return CR_OK;
}

static uint8_t buffer[256 * 256 * 5];
DFhackCExport command_result plugin_onupdate (color_ostream &out)
{
    if (!client->isConnected())
    {
        plugin_enable(out, false);
        out.printerr("yadc: Lost client connection\n");
        return CR_OK;
    }
    static int32_t last_gpu_tick = 0;
    int32_t len;
    if (is_enabled && enabler->gputicks.value != last_gpu_tick)
    {
        last_gpu_tick = enabler->gputicks.value;
        CoreSuspender suspend;
        len = screen::serialize_changed(buffer, sizeof(buffer));
        // Send zero-lengh packets (actually 4 null bytes) about twice a second
        // to allow for connection issues to be detected quickly, even without
        // screen updates or events occurring
        bool ping = (last_gpu_tick % ((int)enabler->gfps / 2) == 0);
        if (len || ping)
        {
            client->send_screen_data(buffer, len);
        }
        len = screen::serialize_events(buffer, sizeof(buffer));
        if (len || ping)
        {
            client->send_comm_data(buffer, len);
        }
    }
    return CR_OK;
}

DFhackCExport command_result plugin_enable (color_ostream &out, bool enable)
{
    if (enable != is_enabled)
    {
        command_result res = enable ? client_connect() : client_disconnect();
        if (res != CR_OK)
        {
            out.printerr("Could not %s client.\n", (enable) ? "start" : "stop");
            return res;
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
        else if (parameters[0] == "reload" || parameters[0] == "reload-config")
            return load_config(out) ? CR_OK : CR_FAILURE;
    }
    else if (parameters.size() == 0)
        return cmd_yadc_status(out);
    return CR_WRONG_USAGE;
}
