#pragma once

#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "DataDefs.h"

#include "modules/Filesystem.h"

#include "jsonxx.h"

#define YADC_CONFIG_PATH "yadc/config.json"

namespace yadc {
    namespace config {
        bool init_config();

        class ConfigParser {
        public:
            ConfigParser (std::string path);
            jsonxx::Object getData() { return data; };
            bool isValid() { return valid; };
        private:
            jsonxx::Object data;
            bool valid;
        };

        struct YADCConfig {
            int comm_port;
            int screen_port;
        };
    }
}
