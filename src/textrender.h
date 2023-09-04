#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include <GLES2/gl2.h>
#include <cstdint>
#include <vector>

class TextRender
{
public:
    TextRender(int width = 1024, int height = 128);

    void clear();

    void putc(int x, int y, char c);

    void printf(int x, int y, const char* fmt, ...);

    void render();

private:
    int m_w, m_h;

    GLuint m_tex = 0;
    GLuint m_vbo = 0;

    std::vector<char> m_lineBuf;

    std::vector<uint8_t> m_textBuf;

    std::vector<uint8_t> m_font;
    uint32_t m_fontWidth;
    uint32_t m_fontHeight;
    uint32_t m_fontCharWidth;
    uint32_t m_fontCharHeight;

};

#endif // TEXTRENDER_H
