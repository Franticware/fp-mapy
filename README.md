# fp-mapy

This program visualizes 3D data from a certain Czech map provider...

https://youtu.be/lZ9HT36tiFc

Press F1 in the program for more information.

## fpmapy.ini
Explanation of some of the configuration values:

| Key           | Description                               | Values                           |
|---------------|-------------------------------------------|----------------------------------|
| \[advanced\]  |                                           |                                  |
| anglePlatform | Backend for ANGLE (if applicable)         | default d3d11 d3d9 opengl vulkan |
| viewDistance  | Max. view range in tiles                  |                                  |
| farPlane      | Max. view range in meters                 |                                  |
| \[location\]  |                                           |                                  |
| pos           | Initial position in tiles                 |                                  |
| rot           | View direction (angles in degrees)        |                                  |

## References

* SDL2, https://www.libsdl.org/
* libcurl, https://curl.se/
* libjpeg, https://libjpeg.sourceforge.net/ (Let's make them happy: This software is based in part on the work of the Independent JPEG Group.)
* libjpeg-turbo, https://libjpeg-turbo.org/
* zlib, https://www.zlib.net/
* ANGLE, https://chromium.googlesource.com/angle/angle
* melowntech repos, https://github.com/melowntech
