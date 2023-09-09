#include "downlthr.h"

#include <future>
#include <thread>
#include <chrono>

DownlThr::DownlThr()
{
}

void DownlThr::setPosRot(const glm::dvec3& pos, const glm::mat3& rot)
{
    m_pos = pos;
    m_rot = rot;
}

void DownlThr::start(MapTileKey key)
{
    m_dl = false;
    m_key = key;
    std::packaged_task<void()> task([this, key] {
        m_result = m_tile.load(key, m_dl);

        const int ok = 0;

        if (m_result == ok)
        {
            m_tile.precomp(m_pos, m_rot);
        }
    });
    m_future = task.get_future();

    m_thread.clear();

    m_thread.push_back(std::thread(std::move(task)));
}

void DownlThr::finish(std::map<MapTileKey, TileGl>& tileGlMap, std::map<MapTileKey, int>& tileStatus, std::map<MapTileKey, int>& tileErrors)
{
    m_thread[0].join();
    m_thread.clear();

    const int maxErrors = 4;

    if (m_result == 0)
    {
        tileGlMap[m_key].load(m_tile);
        tileStatus[m_key] = MTS_DONE;
    }
    else if (m_result == 404)
    {
        tileStatus[m_key] = MTS_404;
    }
    else
    {
        ++tileErrors[m_key];

        if (tileErrors[m_key] > maxErrors)
        {
            tileStatus[m_key] = MTS_FAIL;
        }
        else
        {
            tileStatus[m_key] = MTS_EMPTY;
        }
    }
}

void downlThrEnqueue(std::vector<DownlThr>& downlThr, std::map<MapTileKey, int>& tileStatus, MapTileKey nearestKey)
{
    size_t searchStart = rand() % downlThr.size();
    for (size_t i = 0; i != downlThr.size(); ++i)
    {
        size_t j = (i + searchStart) % downlThr.size();
        if (downlThr[j].m_thread.empty())
        {
            tileStatus[nearestKey] = MTS_IN_PROGRESS;
            downlThr[j].start(nearestKey);
            return;
        }
    }
}

void downlThrDequeue(std::vector<DownlThr>& downlThr, std::map<MapTileKey, TileGl>& tileGlMap, std::map<MapTileKey, int>& tileStatus, std::map<MapTileKey, int>& tileErrors, int* initialLoadCounter, bool& dl)
{
    for (size_t i = 0; i != downlThr.size(); ++i)
    {
        if (!downlThr[i].m_thread.empty())
        {
            using namespace std::chrono_literals;
            auto status = downlThr[i].m_future.wait_for(0ms);
            if (status == std::future_status::ready)
            {
                downlThr[i].finish(tileGlMap, tileStatus, tileErrors);
                if (downlThr[i].m_dl)
                {
                    dl = true;
                }
                if (initialLoadCounter)
                {
                    ++(*initialLoadCounter);
                }
            }
        }
    }
}
