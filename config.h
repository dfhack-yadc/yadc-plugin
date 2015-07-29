#pragma once

#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

#include "modules/Filesystem.h"

#include "jsoncpp.h"

#define YADC_CONFIG_PATH "yadc/config.json"

namespace yadc {
    namespace config {
        bool init_config();

        struct ConfigParser {
            ConfigParser (std::string path);
            Json::Value data;
            bool valid;
        };

        struct YADCConfig {
            int comm_port;
            int screen_port;
            std::string name;
        };
        YADCConfig &get_config();
    }
}
