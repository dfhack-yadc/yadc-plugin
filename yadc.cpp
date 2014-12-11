#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

#include "renderer.h"
#include "server.h"
#include "util.h"

using namespace DFHack;
using namespace df::enums;
using namespace yadc;

DFHACK_PLUGIN("yadc");
DFHACK_PLUGIN_IS_ENABLED(is_enabled);

using df::global::enabler;
using df::global::gps;

static Server* server;
DFhackCExport command_result plugin_init (color_ostream &out, std::vector <PluginCommand> &commands);
DFhackCExport command_result plugin_shutdown (color_ostream &out);
DFhackCExport command_result plugin_onupdate (color_ostream &out);
DFhackCExport command_result plugin_enable (color_ostream &out, bool enable);
command_result cmd_yadc(color_ostream &out, std::vector <std::string> & parameters);

command_result server_start()
{
    if (server)
        return CR_FAILURE;
    server = new Server(25143, 25144);
    command_result res = server->start();
    if (res != CR_OK)
    {
        delete server;
        server = NULL;
    }
    return res;
}

command_result server_stop()
{
    if (!server)
        return CR_FAILURE;
    server->stop();
    delete server;
    server = NULL;
    return CR_OK;
}

DFhackCExport command_result plugin_init (color_ostream &out, std::vector <PluginCommand> &commands)
{
    if (!enabler->renderer->uses_opengl())
    {
        out.printerr("yadc: OpenGL-enabled PRINT_MODE required\n");
        return CR_FAILURE;
    }
    commands.push_back(PluginCommand(
        "yadc", "yadc",
        cmd_yadc, false,
        "  yadc [status]: Display server status\n"
        "  yadc start: Start the server\n"
        "  yadc stop: Stop the server\n"
        "  yadc restart: Restart the server\n"
    ));
    if (getenv("YADC_AUTO_ENABLE"))
    {
        util::print_color(out, COLOR_LIGHTGREEN, "Auto-enabling yadc\n");
        return plugin_enable(out, true);
    }
    return CR_OK;
}

DFhackCExport command_result plugin_shutdown (color_ostream &out)
{
    if (server)
        return server_stop();
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
    if (is_enabled && enabler->gputicks.value != last_gpu_tick)
    {
        last_gpu_tick = enabler->gputicks.value;
        CoreSuspender suspend;
        auto r = static_cast<renderer::YADCRenderer*>(enabler->renderer);
        int32_t len = r->serialize_changed(test_buffer, 256 * 256 * 5);
        if (len)
        {
            server->send_screen_data(test_buffer, len);
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
            res = server_stop();
        else
            res = server_start();
        if (res != CR_OK)
        {
            out.printerr("Could not %s server.\n", (enable) ? "start" : "stop");
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
            "Server is already %s\n", (is_enabled) ? "enabled" : "disabled");
        return CR_FAILURE;
    }
}

command_result cmd_yadc_status(color_ostream &out)
{
    util::print_color(out, (is_enabled) ? COLOR_GREEN : COLOR_RED,
        "Server %s\n", (is_enabled) ? "enabled" : "disabled");
    return CR_OK;
}

command_result cmd_yadc(color_ostream &out, std::vector <std::string> &parameters)
{
    CoreSuspender suspend;
    if (parameters.size() >= 1)
    {
        if (parameters[0] == "start" || parameters[0] == "enable")
        {
            return plugin_enable(out, true);
        }
        else if (parameters[0] == "stop" || parameters[0] == "disable")
        {
            return plugin_enable(out, false);
        }
        else if (parameters[0] == "restart")
        {
            command_result res = plugin_enable(out, false);
            if (res == CR_OK)
            {
                res = plugin_enable(out, true);
            }
            return res;
        }
        else if (parameters[0] == "status")
            return cmd_yadc_status(out);
    }
    else if (parameters.size() == 0)
        return cmd_yadc_status(out);
    return CR_WRONG_USAGE;
}
