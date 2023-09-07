#include <cstdio>
#include <cstdint>

#include <memory>
#include <fstream>
#include <algorithm>
#include <map>

#include <SDL2/SDL.h>
#include <GLES2/gl2.h>

#include "maptile.h"
#include "shadermng.h"
#include "fpcam.h"
#include "downlthr.h"
#include "textrender.h"
#include "meshes.h"
#include "config.h"
#include "singleinst.h"

#include "checkgl.h"

void calcViewportParams(int* x, int* y, int* w, int* h, int ww, int wh, int aw, int ah)
{
    if (ww * ah > wh * aw)
    {
        *w = wh * aw / ah;
        *h = wh;
        *x = (ww - *w) >> 1;
        *y = 0;
    }
    else
    {
        *w = ww;
        *h = ww * ah / aw;
        *x = 0;
        *y = (wh - *h) >> 1;
    }
}

void quitSDLWindow(SDL_Window* w)
{
    SDL_DestroyWindow(w);
    SDL_Quit();
}

int main(int argc, char** argv)
{
    const int ok = 0;

#ifdef __WIN32__
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "system");
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "0");
    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
    SDL_SetHint(SDL_HINT_VIDEO_WIN_D3DCOMPILER, "none");
#endif

    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return 1;
    }

    gConf.init();

    if (singleInst() != ok)
    {
        return 1;
    }

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, int(gConf.depthBits()));

    uint32_t fullscreenFlag = 0;
    if (gConf.fullscreen())
    {
        fullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    uint32_t windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | fullscreenFlag;
    const int windowWidth = gConf.resolutionX();
    const int windowHeight = gConf.resolutionY();
    const int aspectW = gConf.aspectRatioX();
    const int aspectH = gConf.aspectRatioY();
    std::unique_ptr<SDL_Window, decltype(&quitSDLWindow)> window(SDL_CreateWindow(
                                                                     "First Person Mapy",
                                                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                                                     windowWidth, windowHeight, windowFlags),
                                                                 &quitSDLWindow);
    if (!window.get())
    {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GLContext context = SDL_GL_CreateContext(window.get());
    if (!context)
    {
        fprintf(stderr, "Unable to create GL context: %s\n", SDL_GetError());
        return 1;
    }
    int vsync = 0;
    if (gConf.vsync())
    {
        vsync = 1;
    }
    SDL_GL_SetSwapInterval(vsync);
    ShaderMng shaderMng;
    shaderMng.init();
    glClearColor(0.f, 0.5f, 0.f, 0.f); checkGL(__FILE__, __LINE__);
    glEnable(GL_DEPTH_TEST); checkGL(__FILE__, __LINE__);
    glDepthFunc(GL_LEQUAL); checkGL(__FILE__, __LINE__);

    glEnable(GL_CULL_FACE); checkGL(__FILE__, __LINE__);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL(__FILE__, __LINE__);

    TextRender textRender;

    const int confFadeMs = 1000;

    int fadeCounterMs = 0;
    int initialLoadCounter = 0;
    int* initialLoadCounterPtr = &initialLoadCounter;

    glm::mat4 perspectiveMat;
    perspectiveMat = glm::perspective(glm::radians(gConf.fov()), float(aspectW)/float(aspectH), 0.1f, gConf.farPlane());

    glm::mat4 textOrthoMat;

    MeshSkybox skybox;
    MeshFader fader;

    int tilePos0 = gConf.pos0();
    int tilePos1 = gConf.pos1();

    float camRot0 = gConf.rot0();
    float camRot1 = gConf.rot1();

    if (argc == 2)
    {
        TileLocation loc;
        if (getLocationFromIni(argv[1], loc) == ok)
        {
            tilePos0 = loc.pos0;
            tilePos1 = loc.pos1;
            camRot0 = loc.rot0;
            camRot1 = loc.rot1;
        }
    }

    MapTileKey currentKey(tilePos0, tilePos1);

    const float mouseRotSpeed = gConf.mouseSensitivity() * 0.001f;
    const float movementSpeed = gConf.speed();
    const float movementFastSpeed = gConf.shiftSpeed();

    MapTile firstTile;
    if (firstTile.load(currentKey) != ok)
    {
        // first tile must load
        return 1;
    }

    glm::dvec3 basePos;
    glm::mat3 baseRot;
    firstTile.calcPosRot(basePos, baseRot);

    firstTile.precomp(basePos, baseRot);

    std::map<MapTileKey, TileGl> tileGlMap;
    std::map<MapTileKey, int> tileStatus;

    tileGlMap[currentKey].load(firstTile);
    tileStatus[currentKey] = MTS_DONE;

    bool running = true;
    Uint32 ticksPrev = SDL_GetTicks();
    FPCam cam;

    const int confDownlRadius = gConf.viewDistance();
    const int confDropRadius = confDownlRadius + 10;

    // prepare drawing keys from near to far for fillrate optimization
    std::vector<MapTileKey> renderKeys;
    for (int y = -confDownlRadius; y <= confDownlRadius; ++y)
    {
        for (int x = -confDownlRadius; x <= confDownlRadius; ++x)
        {
            MapTileKey key(x, y);
            renderKeys.push_back(key);
        }
    }
    std::stable_sort(renderKeys.begin(), renderKeys.end(), [](const MapTileKey &a, const MapTileKey &b) { return a.x * a.x + a.y * a.y < b.x * b.x + b.y * b.y; });

    const size_t confThreadCount = gConf.parallelDownloads();

    std::vector<DownlThr> downlThr(confThreadCount);
    for (auto& dt : downlThr)
    {
        dt.setPosRot(basePos, baseRot);
    }

    cam.p_pos.x=0;cam.p_pos.y=1.7;cam.p_pos.z=0;cam.p_ay=glm::radians(camRot0);cam.p_ax=glm::radians(camRot1);

    glm::mat4 cullModelViewMat(1.f);

    bool updateCull = true;
    bool showStats = false;

    float currentFps = 0.f;
    uint32_t fpsTimeAccum = 0;
    uint32_t fpsFrameAccum = 0;
    const uint32_t confFpsUpdateInterval = 1000;

    const uint32_t confMaxTilesPerFrame = gConf.maxTilesPerFrame();

    bool keyL = false, keyR = false, keyF = false, keyB = false;
    int mouseDx = 0, mouseDy = 0;
    bool keyFast = false;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                case SDLK_a:
                    keyL = true;
                    break;
                case SDLK_d:
                    keyR = true;
                    break;
                case SDLK_w:
                    keyF = true;
                    break;
                case SDLK_s:
                    keyB = true;
                    break;
                case SDLK_LSHIFT:
                    keyFast = true;
                    break;
                case SDLK_RETURN:
                    if (event.key.keysym.mod & KMOD_ALT)
                    {
                        if (fullscreenFlag == 0)
                        {
                            fullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
                        }
                        else
                        {
                            fullscreenFlag = 0;
                        }
                        SDL_SetWindowFullscreen(window.get(), fullscreenFlag);
                    }
                    break;
                case SDLK_F1:
                    showStats = !showStats;
                    break;
                case SDLK_F2:
                    updateCull = !updateCull;
                    break;
                default:
                    break;
                }
            }
            else if (event.type == SDL_KEYUP)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_a:
                    keyL = false;
                    break;
                case SDLK_d:
                    keyR = false;
                    break;
                case SDLK_w:
                    keyF = false;
                    break;
                case SDLK_s:
                    keyB = false;
                    break;
                case SDLK_LSHIFT:
                    keyFast = false;
                    break;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                }
                else if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                }
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                if (SDL_GetRelativeMouseMode())
                {
                    mouseDx += event.motion.xrel;
                    mouseDy += event.motion.yrel;
                }
            }
            else if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }
        if (running)
        {
            uint32_t ticksNow = SDL_GetTicks();
            uint32_t dt = ticksNow - ticksPrev;
            ticksPrev = ticksNow;

            {
                fpsTimeAccum += dt;
                ++fpsFrameAccum;
                if (fpsTimeAccum >= confFpsUpdateInterval)
                {
                    currentFps = 1000.f * float(fpsFrameAccum)/float(fpsTimeAccum);
                    fpsFrameAccum = 0;
                    fpsTimeAccum = 0;
                }
            }
            cam.mouse(mouseDx, mouseDy, mouseRotSpeed);
            mouseDx = 0;
            mouseDy = 0;
            const float moveSpeed = keyFast ? movementFastSpeed : movementSpeed; // m/s
            cam.keyb(keyL, keyR, keyF, keyB, dt * 0.001 * moveSpeed, dt * 0.001 * moveSpeed);

            if (initialLoadCounter > int(gConf.fadeInTileCount()))
            {
                initialLoadCounterPtr = nullptr;
            }
            if (initialLoadCounterPtr == nullptr && fadeCounterMs < confFadeMs)
            {
                fadeCounterMs += dt;
            }

            //bool keyUpdated = false;
            {
                //const auto prevKey = currentKey;
                float currentDist = tileGlMap[currentKey].dist(cam.p_pos);
                // update current tile
                for (int y = -1; y <= 1; ++y)
                {
                    for (int x = -1; x <= 1; ++x)
                    {
                        MapTileKey key(currentKey.x + x, currentKey.y + y);

                        if (tileGlMap.find(key) != tileGlMap.end())
                        {
                            float dist = tileGlMap[key].dist(cam.p_pos);
                            if (dist < currentDist)
                            {
                                currentKey = key;
                                currentDist = dist;
                            }
                        }
                    }
                }
                //if (prevKey != currentKey)
                //{
                //    keyUpdated = true;
                //}
            }

            for (uint32_t t = 0; t < confMaxTilesPerFrame; ++t)
            {
                bool nearestFound = false;
                auto nearestKey = currentKey;
                {
                    int nearestDist = confDownlRadius * 3;
                    for (int y = -confDownlRadius; y <= confDownlRadius; ++y)
                    {
                        for (int x = -confDownlRadius; x <= confDownlRadius; ++x)
                        {
                            MapTileKey key(currentKey.x + x, currentKey.y + y);
                            int dist = key.dist(currentKey);
                            if (dist < nearestDist)
                            {
                                auto it = tileStatus.find(key);
                                if (it == tileStatus.end() || it->second == MTS_EMPTY)
                                {
                                    nearestFound = true;
                                    nearestKey = key;
                                    nearestDist = dist;
                                }
                            }
                        }
                    }
                }

                if (nearestFound)
                {
                    downlThrEnqueue(downlThr, tileStatus, nearestKey);
                }
            }

            int x, y, w, h;
            SDL_GL_GetDrawableSize(window.get(), &w, &h);
            calcViewportParams(&x, &y, &w, &h, w, h, aspectW, aspectH);
            glViewport(x, y, w, h); checkGL(__FILE__, __LINE__);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); checkGL(__FILE__, __LINE__); // clears the whole drawable, not just glViewport area

            textOrthoMat = glm::ortho(0.f, float(w), float(h), 0.f);

            glm::mat4 modelViewMat = cam.transf();

            shaderMng.set(ShaderUniMat4::ProjMat, perspectiveMat);
            shaderMng.set(ShaderUniMat4::ModelViewMat, modelViewMat);
            shaderMng.use(ShaderId::Tex);

            /*for (const auto& tilePair : tileGlMap) // naive rendering
            {
                const auto& tile = tilePair.second;
                tile.render();
            }*/

            if (updateCull)
            {
                cullModelViewMat = modelViewMat;
            }

            for (const auto& rk : renderKeys) // render front to back, only tiles intersecting frustum
            {
                MapTileKey key(currentKey.x + rk.x, currentKey.y + rk.y);
                auto it = tileGlMap.find(key);
                if (it != tileGlMap.end())
                {
                    if (it->second.cullFrustum(perspectiveMat * cullModelViewMat))
                    {
                        it->second.render();
                    }
                }
            }

            skybox.render(shaderMng);

            if (fadeCounterMs < confFadeMs)
            {
                const float fadeAlpha = float(confFadeMs - fadeCounterMs) / confFadeMs;
                fader.render(shaderMng, fadeAlpha);
            }

            if (initialLoadCounterPtr)
            {
                shaderMng.set(ShaderUniMat4::ProjMat, textOrthoMat);
                shaderMng.set(ShaderUniMat4::ModelViewMat, glm::mat4(1.f));
                shaderMng.use(ShaderId::ColorTex);
                textRender.clear();
                textRender.printf(0, 0, "Loading...");
                textRender.render();
            }
            else if (showStats)
            {
                shaderMng.set(ShaderUniMat4::ProjMat, textOrthoMat);
                shaderMng.set(ShaderUniMat4::ModelViewMat, glm::mat4(1.f));
                shaderMng.use(ShaderId::ColorTex);
                textRender.clear();
                int line = 0;
                textRender.printf(0, line++, "%.2f fps", currentFps);
                textRender.printf(0, line++, "pos %d %d | rot %.2f %.2f", currentKey.x, currentKey.y, glm::degrees(cam.p_ay), glm::degrees(cam.p_ax));
                textRender.printf(0, line++, "config path '%s'", gConf.configPath().c_str());
                line++;
                textRender.printf(0, line++, "WASD - movement");
                textRender.printf(0, line++, "left shift - fast movement");
                textRender.printf(0, line++, "left click - capture mouse, start mouse look");
                textRender.printf(0, line++, "right click - release mouse");
                textRender.printf(0, line++, "F1 - show info");
                textRender.printf(0, line++, "F2 - freeze culling matrix");
                textRender.printf(0, line++, "Alt + Enter - toggle fullscreen");
                textRender.printf(0, line++, "Esc - quit");
                line++;
                textRender.printf(0, line++, "https://github.com/Franticware/fp-mapy");

                // show text area width in characters
                //textRender.printf(0, 0, "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111", configFile.m_configPath.c_str());
                //textRender.printf(0, 1, "00000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999", configFile.m_configPath.c_str());
                //textRender.printf(0, 2, "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", configFile.m_configPath.c_str());

                textRender.render();
            }

            SDL_GL_SwapWindow(window.get());

            for (uint32_t t = 0; t < confMaxTilesPerFrame; ++t)
            {
                downlThrDequeue(downlThr, tileGlMap, tileStatus, initialLoadCounterPtr);
            }
            for (uint32_t t = 0; t < confMaxTilesPerFrame; ++t)
            {

                for (auto it = tileGlMap.begin(); it != tileGlMap.end(); ++it)
                {
                    if (std::abs(it->first.x - currentKey.x) > confDropRadius || std::abs(it->first.y - currentKey.y) > confDropRadius)
                    {
                        tileGlMap.erase(it);
                        auto its = tileStatus.find(it->first);
                        if (its != tileStatus.end())
                        {
                            tileStatus.erase(its);
                        }
                        break;
                    }
                }
            }
        }
    }

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); checkGL(__FILE__, __LINE__);
    SDL_GL_SwapWindow(window.get());

    SDL_HideWindow(window.get()); // hide window immediately after user quits

    // let all active threads finish (might take a second or two)
    for (auto& dt : downlThr)
    {
        if (!dt.m_thread.empty())
        {
            dt.m_thread[0].join();
        }
    }

    return 0;
}
