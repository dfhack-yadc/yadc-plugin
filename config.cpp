#include "config.h"

using namespace DFHack;
using namespace yadc;
using namespace yadc::config;

bool config::init_config()
{
    if (Filesystem::exists("yadc") && !Filesystem::isdir("yadc"))
        return false;
    if (Filesystem::isdir("yadc") || Filesystem::mkdir("yadc"))
    {
        if (!Filesystem::exists(YADC_CONFIG_PATH))
        {
            std::string contents = "{}";
            std::ofstream out(YADC_CONFIG_PATH);
            out << contents;
            out.close();
            if (!Filesystem::exists(YADC_CONFIG_PATH))
                return false;
        }
        return true;
    }
    return false;
}

ConfigParser::ConfigParser (std::string path)
    :valid(false)
{
    std::ifstream in(path);
    if (in) {
        std::string contents;
        in.seekg(0, std::ios_base::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios_base::beg);
        in.read(&contents[0], contents.size());
        in.close();
        valid = data.parse(contents);
    }
}
