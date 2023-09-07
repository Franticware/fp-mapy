TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lSDL2 -lGLESv2 -ljpeg -lz -lcurl

SOURCES += \
        config.cpp \
        culling.cpp \
        downl.cpp \
        downlthr.cpp \
        fpcam.cpp \
        freespace.cpp \
        load_texture.cpp \
        main.cpp \
        maptile.cpp \
        meshes.cpp \
        pict.cpp \
        pict_jpeg.cpp \
        shadermng.cpp \
        singleinst.cpp \
        skyboxdata.cpp \
        textrender.cpp \
        vts/mesh.cpp \
        vts/meshgz.cpp \
        vts/meshio.cpp

HEADERS += \
    checkgl.h \
    config.h \
    culling.h \
    downl.h \
    downlthr.h \
    fpcam.h \
    freespace.h \
    load_texture.h \
    maptile.h \
    meshes.h \
    mini/ini.h \
    pict.h \
    shadermng.h \
    shaders/color_tex.fs.h \
    shaders/color_tex.vs.h \
    shaders/sky.fs.h \
    shaders/sky.vs.h \
    shaders/sky_compat.fs.h \
    shaders/sky_compat.vs.h \
    shaders/tex.fs.h \
    shaders/tex.vs.h \
    singleinst.h \
    skyboxdata.h \
    textrender.h \
    vts/binaryio.hpp \
    vts/det_streams.hpp \
    vts/enum-io.hpp \
    vts/expect.hpp \
    vts/filetypes.hpp \
    vts/format.hpp \
    vts/gccversion.hpp \
    vts/geometry.hpp \
    vts/geometry_core.hpp \
    vts/math.hpp \
    vts/mesh.hpp \
    vts/meshgz.h \
    vts/meshio.hpp \
    vts/raise.hpp \
    vts/streams.hpp

DISTFILES += \
    todo.txt
