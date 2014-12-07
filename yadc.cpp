#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

#include "server.h"
#include "util.h"

using namespace DFHack;
using namespace df::enums;
using namespace yadc;

DFHACK_PLUGIN("yadc");
DFHACK_PLUGIN_IS_ENABLED(is_enabled);

command_result cmd_yadc(color_ostream &out, std::vector <std::string> & parameters);

DFhackCExport command_result plugin_init (color_ostream &out, std::vector <PluginCommand> &commands)
{
    commands.push_back(PluginCommand(
        "yadc", "yadc",
        cmd_yadc, false,
        "  yadc start: Start the server\n"
        "  yadc stop: Stop the server\n"
    ));
    return CR_OK;
}

DFhackCExport command_result plugin_shutdown (color_ostream &out)
{
    return CR_OK;
}


DFhackCExport command_result plugin_onupdate (color_ostream &out)
{
    return CR_OK;
}

DFhackCExport command_result plugin_enable (color_ostream &out, bool enable)
{
    if (enable != is_enabled)
    {
        command_result res;
        if (is_enabled)
            res = Server::stop();
        else
            res = Server::start();
        if (res != CR_OK)
            out.printerr("Could not %s server.\n", (enable) ? "enable" : "disable");
        else
            is_enabled = enable;
        return res;
    }
    else {
        util::print_color(out, (is_enabled) ? COLOR_LIGHTGREEN : COLOR_YELLOW,
            "Server is already %s\n", (is_enabled) ? "enabled" : "disabled");
        return CR_FAILURE;
    }
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
    }
    return CR_WRONG_USAGE;
}
