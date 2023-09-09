#ifndef DOWNLTHR_H
#define DOWNLTHR_H

#include <thread>
#include <vector>
#include <map>
#include <future>

#include "maptile.h"

#define DTS_FREE 0
#define DTS_IN_PROGRESS 1
#define DTS_DONE 2

class DownlThr
{
public:
    DownlThr();

    void setPosRot(const glm::dvec3& pos, const glm::mat3& rot);

    void start(MapTileKey key);
    void finish(std::map<MapTileKey, TileGl>& tileGlMap, std::map<MapTileKey, int>& tileStatus, std::map<MapTileKey, int>& tileErrors);

    bool m_dl = false;

    std::vector<std::thread> m_thread;

    std::future<void> m_future;

    int m_state = DTS_FREE;

    MapTileKey m_key = MapTileKey(0, 0);


    glm::dvec3 m_pos;
    glm::mat3 m_rot;

    MapTile m_tile;
    int m_result = 0;
};

void downlThrEnqueue(std::vector<DownlThr>& downlThr, std::map<MapTileKey, int>& tileStatus, MapTileKey nearestKey);
void downlThrDequeue(std::vector<DownlThr>& downlThr, std::map<MapTileKey, TileGl>& tileGlMap, std::map<MapTileKey, int>& tileStatus, std::map<MapTileKey, int>& tileErrors, int* initialLoadCounter, bool& dl);

#endif // DOWNLTHR_H
