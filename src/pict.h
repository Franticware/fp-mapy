#ifndef HLIDAC_PICT_H
#define HLIDAC_PICT_H

#include <cstring>
#include <vector>
#include <cstdint>
#include <algorithm>

class Pict {
public:
    Pict() : p_w(0), p_h(0) {}
    explicit Pict(int prm_w, int prm_h, const unsigned char* prm_px)
    {
        create(prm_w, prm_h, prm_px);
    }
    int loadjpeg(const char*);
    int loadjpeg(const void*, unsigned int);
    void clear() { p_px.clear(); p_w = 0; p_h = 0; }
    bool empty() const
    {
        return !(p_w && p_h && !p_px.empty());
    }
    int w() const { return p_w; }
    int h() const { return p_h; }
    uint8_t* px() { return p_px.data(); }
    const unsigned char* c_px() const { return p_px.data(); }
private:
    int p_w, p_h; // private width, height, depth
    std::vector<uint8_t> p_px; // pixels
    void create(int prm_w, int prm_h, const unsigned char* prm_px)
    {
        p_w = prm_w; p_h = prm_h;
        p_px.resize(p_w*p_h*3);
        if (prm_px) std::copy_n(prm_px, p_px.size(), p_px.begin());
    }
    int loadjpeg_pom(bool bfile, const void* fname_data, unsigned int data_size);
};

#endif
