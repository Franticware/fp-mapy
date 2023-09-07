#include "textrender.h"
#include "shadermng.h"

#include <cstdlib>
#include <cstdarg>
#include <cstdio>

#include "checkgl.h"

TextRender::TextRender(int width, int height) : m_w(width), m_h(height)
{
    // https://opengameart.org/content/pixel-font-basic-latin-latin-1-box-drawing
    static const uint32_t fontData[256]={
        0,0,0,0,0x82850400,0x10103010,2,0x2000,0xc2850400,0x810492b,0x10204,0x2000,0xa7c00400,0x8002890,0x10a84,0x1000,0xc2800400,0x8001041,0xf007c704,0x1001,0x87c00400,0x800a922,0x10a84,0x1000,0xe2800000,
        0x8004a91,0x410204,0x800,0x82800400,0x1000b100,0x400002,0x810,0,0,0x200000,0,0,0,0,0,0x8387041c,0x1c7c71f1,0xe,0x3800,0x44488622,0x22400811,0x1008111,0x4410,0x24080532,0x22200811,0xf0800011,0x4421,
        0xe304042a,0x1c1078f3,0x40001e,0x2040,0x4020426,0x22088901,0xf0800010,0x1021,0x4410422,0x22088901,0x1008110,0x10,0x38f9f1c,0x1c0870f1,0x800e,0x1000,0,0,0x4000,0,0,0,0,0,0xe3878e1c,0x2238f9f1,
        0x10244f9f,0x3889,0x24489122,0x22440812,0xb0244804,0x4489,0x2048913a,0x22040812,0x50224804,0x4499,0x20479f2a,0x3e0478f2,0x1021c804,0x44a9,0x2048913a,0x22640812,0x10224804,0x44c9,0x24489102,0x22440812,
        0x10244884,0x4489,0xe387911c,0x223809f1,0x13e4471f,0x3889,0,0,0,0,0,0,0,0,0xe7878e1e,0x22448913,0x60430f91,0x20,0x80489122,0x22448910,0x40410811,0x50,0x80489122,0x14448910,0x4081040a,0x88,0x8387911e,
        0x8448910,0x40810204,0,0x84089502,0x14548910,0x40810104,0,0x84088902,0x226c5110,0x41010084,0,0x83c89602,0x224420e0,0x61030f84,0x7c00,0,0,0,0,0,0,0,0,0x8004,0x2006002,0x204804,0,0x8008,0x2001002,
        0x204000,0,0xc3878e00,0x1e78f8e3,0xb0224e07,0x3878,0x24489000,0x22441112,0x5021c804,0x4489,0x20489e00,0x224411f2,0x50224804,0x4489,0x24489100,0x22781012,0x11244884,0x4489,0xc3879e00,0x224011e3,
        0x10c4471f,0x3889,0,0x380000,0,0,0,0,0,0,0x40000000,0,0x20820000,0,0x40000000,0,0x40810000,0,0xe7869e1e,0x22448911,0x40810f91,0x10,0x40499122,0x14448910,0x80808411,0xa8,0x43809122,0x8448910,
        0x4081021e,0x40,0x44009e1e,0x14545112,0x40810110,0,0x83c09002,0x222821e1,0x20820f8e,0,0x1002,
    };
    m_fontWidth = 128;
    m_fontHeight = 64;
    m_fontCharWidth = 7;
    m_fontCharHeight = 9;
    m_font.resize(m_fontWidth * m_fontHeight);
    for (uint32_t y = 0; y != m_fontHeight; ++y)
    {
        for (uint32_t x = 0; x != m_fontWidth; ++x)
        {
            m_font[x + y * m_fontWidth] = 255 * !!(fontData[(x / 32) + y * (m_fontWidth/32)] & (1 << (x & 31)));
        }
    }

    m_textBuf.resize(m_w * m_h, 0);

    m_lineBuf.resize(m_w/m_fontCharWidth + 2, 0);

    /*for (int i = 0; i != m_w * m_h; ++i)
    {
        m_textBuf[i] = (rand() % 5) > 3 ? 255 : 0;
    }*/

    glGenTextures(1, &m_tex); checkGL(__FILE__, __LINE__);
    glBindTexture(GL_TEXTURE_2D, m_tex); checkGL(__FILE__, __LINE__);
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, m_w, m_h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, m_textBuf.data()); checkGL(__FILE__, __LINE__);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); checkGL(__FILE__, __LINE__);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); checkGL(__FILE__, __LINE__);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); checkGL(__FILE__, __LINE__);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); checkGL(__FILE__, __LINE__);
    }
    glBindTexture(GL_TEXTURE_2D, 0); checkGL(__FILE__, __LINE__);



    //float maxZ = 1;
    //float minZ = -1;

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    const float xOff0 = 4;
    const float yOff0 = 4;

    const float xOffp = xOff0 + 1;
    const float yOffp = yOff0 + 1;

    const float xOffn = xOff0 - 1;
    const float yOffn = yOff0 - 1;

    static const float fadeData[6 * 9 * 5] = {
        /* */ 0 + xOffn,       0 + yOff0, 0,       0, 0,  0, 0, 0, 1,
        /* */ 0 + xOffn, m_h * 2 + yOff0, 0,       0, 1,  0, 0, 0, 1,
        m_w * 2 + xOffn, m_h * 2 + yOff0, 0,       1, 1,  0, 0, 0, 1,
        /* */ 0 + xOffn,       0 + yOff0, 0,       0, 0,  0, 0, 0, 1,
        m_w * 2 + xOffn, m_h * 2 + yOff0, 0,       1, 1,  0, 0, 0, 1,
        m_w * 2 + xOffn,       0 + yOff0, 0,       1, 0,  0, 0, 0, 1,
        /* */ 0 + xOffp,       0 + yOff0, 0,       0, 0,  0, 0, 0, 1,
        /* */ 0 + xOffp, m_h * 2 + yOff0, 0,       0, 1,  0, 0, 0, 1,
        m_w * 2 + xOffp, m_h * 2 + yOff0, 0,       1, 1,  0, 0, 0, 1,
        /* */ 0 + xOffp,       0 + yOff0, 0,       0, 0,  0, 0, 0, 1,
        m_w * 2 + xOffp, m_h * 2 + yOff0, 0,       1, 1,  0, 0, 0, 1,
        m_w * 2 + xOffp,       0 + yOff0, 0,       1, 0,  0, 0, 0, 1,
        /* */ 0 + xOff0,       0 + yOffn, 0,       0, 0,  0, 0, 0, 1,
        /* */ 0 + xOff0, m_h * 2 + yOffn, 0,       0, 1,  0, 0, 0, 1,
        m_w * 2 + xOff0, m_h * 2 + yOffn, 0,       1, 1,  0, 0, 0, 1,
        /* */ 0 + xOff0,       0 + yOffn, 0,       0, 0,  0, 0, 0, 1,
        m_w * 2 + xOff0, m_h * 2 + yOffn, 0,       1, 1,  0, 0, 0, 1,
        m_w * 2 + xOff0,       0 + yOffn, 0,       1, 0,  0, 0, 0, 1,
        /* */ 0 + xOff0,       0 + yOffp, 0,       0, 0,  0, 0, 0, 1,
        /* */ 0 + xOff0, m_h * 2 + yOffp, 0,       0, 1,  0, 0, 0, 1,
        m_w * 2 + xOff0, m_h * 2 + yOffp, 0,       1, 1,  0, 0, 0, 1,
        /* */ 0 + xOff0,       0 + yOffp, 0,       0, 0,  0, 0, 0, 1,
        m_w * 2 + xOff0, m_h * 2 + yOffp, 0,       1, 1,  0, 0, 0, 1,
        m_w * 2 + xOff0,       0 + yOffp, 0,       1, 0,  0, 0, 0, 1,

        /* */ 0 + xOff0,       0 + yOff0, 0,       0, 0,  1, 1, 1, 1,
        /* */ 0 + xOff0, m_h * 2 + yOff0, 0,       0, 1,  1, 1, 1, 1,
        m_w * 2 + xOff0, m_h * 2 + yOff0, 0,       1, 1,  1, 1, 1, 1,
        /* */ 0 + xOff0,       0 + yOff0, 0,       0, 0,  1, 1, 1, 1,
        m_w * 2 + xOff0, m_h * 2 + yOff0, 0,       1, 1,  1, 1, 1, 1,
        m_w * 2 + xOff0,       0 + yOff0, 0,       1, 0,  1, 1, 1, 1,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(fadeData), fadeData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TextRender::printf(int x, int y, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(m_lineBuf.data(), m_lineBuf.size() - 1, format, args);
    va_end(args);

    for (int i = 0; i != int(strlen(m_lineBuf.data())); ++i)
    {
        putc(x + i, y, m_lineBuf[i]);
    }
}

void TextRender::clear()
{
    memset(m_textBuf.data(), 0, m_textBuf.size());
}

void TextRender::putc(int xpos, int ypos, char c)
{
    uint8_t uc = c;
    if (uc < 32)
    {
        uc = 128;
    }
    if (uc > 128)
    {
        uc = 128;
    }
    uc -= 32;

    int ucx = uc & 15;
    int ucy = uc >> 4;

    // pixel-perfect up until the right edge
    /*for (int y = 0; y != m_fontCharHeight; ++y)
    {
        for (int x = 0; x != m_fontCharWidth; ++x)
        {
            int x_ = x + xpos * m_fontCharWidth;
            int y_ = y + ypos * m_fontCharHeight;
            if (x_ < m_w && y < m_h)
            {
                m_textBuf[x_ + y_ * m_w] = m_font[x + ucx * m_fontCharWidth + (y + ucy * m_fontCharHeight) * m_fontWidth];
            }
        }
    }*/

    // fast
    for (int y = 0; y != int(m_fontCharHeight); ++y)
    {
        int y_ = y + ypos * m_fontCharHeight;
        if (int((xpos + 1) * m_fontCharWidth) < m_w && y_ < m_h)
        {
            memcpy(m_textBuf.data() + xpos * m_fontCharWidth + y_ * m_w, m_font.data() + ucx * m_fontCharWidth + (y + ucy * m_fontCharHeight) * m_fontWidth, m_fontCharWidth);
        }
    }
}

void TextRender::render()
{
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_w, m_h, GL_ALPHA, GL_UNSIGNED_BYTE, m_textBuf.data()); checkGL(__FILE__, __LINE__);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL(__FILE__, __LINE__);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL(__FILE__, __LINE__);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Color); checkGL(__FILE__, __LINE__);
    glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*9, 0);
    glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*9, (void*)(sizeof(float)*3));
    glVertexAttribPointer((GLuint)ShaderAttrib::Color, 4, GL_FLOAT, GL_FALSE, sizeof(float)*9, (void*)(sizeof(float)*5));
    glDrawArrays(GL_TRIANGLES, 0, 6 * 5);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL(__FILE__, __LINE__);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL(__FILE__, __LINE__);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Color); checkGL(__FILE__, __LINE__);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
