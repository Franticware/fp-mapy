#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <cstdint>

#define MINI_CASE_SENSITIVE
#include "mini/ini.h"

class Config
{
public:
    Config();

    void init();

    const std::string& configPath() const { return m_configPath; }
    const char* configPathCstr() const { return m_configPathCstr; }

    // basic
    uint32_t resolutionX() const { return v_resolution[0]; }
    uint32_t resolutionY() const { return v_resolution[1]; }
    uint32_t fullscreen() const { return v_fullscreen; }
    float mouseSensitivity() const { return v_mouseSensitivity; }
    uint32_t pos0() const { return v_pos[0]; }
    uint32_t pos1() const { return v_pos[1]; }
    float rot0() const { return v_rot[0]; }
    float rot1() const { return v_rot[1]; }

    // advanced
    const std::string& anglePlatform() const { return v_anglePlatform; }
    uint32_t vsync() const { return v_vsync; }
    uint32_t aspectRatioX() const { return v_aspectRatio[0]; }
    uint32_t aspectRatioY() const { return v_aspectRatio[1]; }
    uint32_t depthBits() const { return v_depthBits; }
    float farPlane() const { return v_farPlane; }
    float fov() const { return v_fov; }
    float speed() const { return v_speed; }
    float shiftSpeed() const { return v_shiftSpeed; }
    uint32_t viewDistance() const { return v_viewDistance; }
    uint32_t maxTilesPerFrame() const { return v_maxTilesPerFrame; }
    uint32_t fadeInTileCount() const { return v_fadeInTileCount; }
    const std::string& baseUrl() const { return v_baseUrl; }
    const char* baseUrlCstr() const { return v_baseUrlCstr; }
    uint32_t parallelDownloads() const { return v_parallelDownloads; }

    // cache
    uint32_t cacheEnabled() const { return v_cacheEnabled; }
    const std::string& cachePath() const { return v_cachePath; }
    const char* cachePathCstr() const { return v_cachePathCstr; }
    uint32_t cacheMinFreeSpaceGB() const { return v_cacheMinFreeSpaceGB; }

private:
    bool m_updated = false;
    bool m_init = false;

    template <class T> void sync(mINI::INIStructure& ini, T& val, const std::string& section, const std::string& key)
    {
        if (ini.has(section) && ini[section].has(key))
        {
            sync(ini, val, section, key, true);
        }
        else
        {
            m_updated = true;
            sync(ini, val, section, key, false);
        }
    }

    template <class T> void sync(mINI::INIStructure& ini, T* val, const std::string& section, const std::string& key)
    {
        if (ini.has(section) && ini[section].has(key))
        {
            sync(ini, val, section, key, true);
        }
        else
        {
            m_updated = true;
            sync(ini, val, section, key, false);
        }
    }

    void sync(mINI::INIStructure& ini, uint32_t& val, const std::string& section, const std::string& key, bool rd);
    void sync(mINI::INIStructure& ini, uint32_t val[2], const std::string& section, const std::string& key, bool rd);
    void sync(mINI::INIStructure& ini, float& val, const std::string& section, const std::string& key, bool rd);
    void sync(mINI::INIStructure& ini, float val[2], const std::string& section, const std::string& key, bool rd);
    void sync(mINI::INIStructure& ini, std::string& s, const std::string& section, const std::string& key, bool rd);

    std::string m_configPath;
    const char* m_configPathCstr = nullptr;

    // basic
    uint32_t v_resolution[2] = {1280, 720};
    uint32_t v_fullscreen = 0;
    float v_mouseSensitivity = 2.5f;
    uint32_t v_pos[2] = {566292, 355222};
    float v_rot[2] = {-150.f, 16.f};

    // advanced
    std::string v_anglePlatform = "default";
    uint32_t v_vsync = 1;
    uint32_t v_aspectRatio[2] = {16, 9};
    uint32_t v_depthBits = 24;
    float v_farPlane = 1000.f;
    float v_fov = 50.f;
    float v_speed = 16.f;
    float v_shiftSpeed = 64.f;
    uint32_t v_viewDistance = 15;
    uint32_t v_maxTilesPerFrame = 1;
    uint32_t v_fadeInTileCount = 50;
    std::string v_baseUrl = "https://mapserver-3d.mapy.cz/latestStage/tilesets/cities/";
    //                      "https://mapserver-3d.mapy.cz/latestStage/tilesets/ev2@3/";
    const char* v_baseUrlCstr = nullptr;
    uint32_t v_parallelDownloads = 4;

    // cache
    uint32_t v_cacheEnabled = 1;
    std::string v_cachePath; // by constructor
    const char* v_cachePathCstr = nullptr;
    uint32_t v_cacheMinFreeSpaceGB = 10;
};

extern Config gConf;

struct TileLocation
{
    int pos0, pos1;
    float rot0, rot1;
};

int getLocationFromIni(const char* iniFile, TileLocation& ret);

#endif // CONFIG_H
