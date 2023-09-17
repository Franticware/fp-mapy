#include "config.h"

#include <SDL2/SDL.h>

#include <cstdio>

#define PREF_ORG "Franticware"
#define PREF_APP "fpmapy"
#define CONFIG_NAME "fpmapy.ini"

Config gConf; // config singleton

Config::Config()
{
}

void  Config::init()
{
    m_init = true;

    m_configPath = SDL_GetPrefPath(PREF_ORG, PREF_APP);
    m_configPathCstr = m_configPath.c_str();
    mINI::INIFile file(m_configPath + CONFIG_NAME);
    mINI::INIStructure ini;
    file.read(ini);
    v_cachePath = SDL_GetPrefPath(PREF_ORG, PREF_APP " cache");
    v_cachePathCstr = v_cachePath.c_str();

    // basic
    sync(ini, v_resolution, "basic", "resolution");
    sync(ini, v_fullscreen, "basic", "fullscreen");
    sync(ini, v_mouseSensitivity, "basic", "mouseSensitivity");

    // advanced
    sync(ini, v_anglePlatform, "advanced", "anglePlatform");
    sync(ini, v_vsync, "advanced", "vsync");
    sync(ini, v_aspectRatio, "advanced", "aspectRatio");
    sync(ini, v_depthBits, "advanced", "depthBits");
    sync(ini, v_farPlane, "advanced", "farPlane");
    sync(ini, v_fov, "advanced", "fov");
    sync(ini, v_speed, "advanced", "speed");
    sync(ini, v_shiftSpeed, "advanced", "shiftSpeed");
    sync(ini, v_viewDistance, "advanced", "viewDistance");
    sync(ini, v_maxTilesPerFrame, "advanced", "maxTilesPerFrame");
    sync(ini, v_fadeInTileCount, "advanced", "fadeInTileCount");
    sync(ini, v_baseUrl, "advanced", "baseUrl");
    sync(ini, v_parallelDownloads, "advanced", "parallelDownloads");

    // cache
    sync(ini, v_cacheEnabled, "cache", "enabled");
    sync(ini, v_cachePath, "cache", "path");
    sync(ini, v_cacheMinFreeSpaceGB, "cache", "minFreeSpaceGB");

    // location
    sync(ini, v_pos, "location", "pos");
    sync(ini, v_rot, "location", "rot");

    if (m_updated)
    {
        file.write(ini);
    }

    v_baseUrlCstr = v_baseUrl.c_str();
}

void Config::sync(mINI::INIStructure& ini, uint32_t& val, const std::string& section, const std::string& key, bool rd)
{
    if (rd)
    {
        sscanf(ini[section][key].c_str(), "%u", &val);
    }
    else
    {
        ini[section][key] = std::to_string(val);
    }
}

void Config::sync(mINI::INIStructure& ini, uint32_t val[2], const std::string& section, const std::string& key, bool rd)
{
    if (rd)
    {
        sscanf(ini[section][key].c_str(), "%u %u", &val[0], &val[1]);
    }
    else
    {
        ini[section][key] = std::to_string(val[0]) + " " + std::to_string(val[1]);
    }
}

void Config::sync(mINI::INIStructure& ini, float& val, const std::string& section, const std::string& key, bool rd)
{
    if (rd)
    {
        sscanf(ini[section][key].c_str(), "%f", &val);
    }
    else
    {
        ini[section][key] = std::to_string(val);
    }
}

void Config::sync(mINI::INIStructure& ini, float val[2], const std::string& section, const std::string& key, bool rd)
{
    if (rd)
    {
        sscanf(ini[section][key].c_str(), "%f %f", &val[0], &val[1]);
    }
    else
    {
        ini[section][key] = std::to_string(val[0]) + " " + std::to_string(val[1]);
    }
}

void Config::sync(mINI::INIStructure& ini, std::string& s, const std::string& section, const std::string& key, bool rd)
{
    if (rd)
    {
        s = ini[section][key];
    }
    else
    {
        ini[section][key] = s;
    }
}

int getLocationFromIni(const char* iniFile, TileLocation& ret)
{
    const int ok = 0;
    const int err = 1;

    mINI::INIFile file(iniFile);
    mINI::INIStructure ini;
    file.read(ini);

    if (sscanf(ini["location"]["pos"].c_str(), "%u %u", &ret.pos0, &ret.pos1) != 2)
        return err;

    if (sscanf(ini["location"]["rot"].c_str(), "%f %f", &ret.rot0, &ret.rot1) != 2)
        return err;

    return ok;
}
