#include "meshgz.h"
#include "meshio.hpp"

#define ZLIB_CONST
#include <zlib.h>

struct membuf : std::streambuf
{
    membuf(char *begin, char *end) : begin(begin), end(end)
    {
        this->setg(begin, begin, end);
    }

    virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in) override
    {
        (void)which;
        if (dir == std::ios_base::cur)
            gbump(off);
        else if(dir == std::ios_base::end)
            setg(begin, end+off, end);
        else if(dir == std::ios_base::beg)
            setg(begin, begin+off, end);

        return gptr() - eback();
    }

    virtual pos_type seekpos(std::streampos pos, std::ios_base::openmode mode) override
    {
        return seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);
    }

    char *begin, *end;
};

int inf(FILE *source, std::vector<uint8_t>& data)
{
    const int CHUNK = 16384;
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char inBuff[CHUNK];
    unsigned char outBuff[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    //ret = inflateInit(&strm); // deflate
    ret = inflateInit2(&strm, 16+MAX_WBITS); // gzip
    if (ret != Z_OK)
    return ret;

    /* decompress until deflate stream ends or end of file */
    do {
    strm.avail_in = fread(inBuff, 1, CHUNK, source);
    if (ferror(source)) {
    (void)inflateEnd(&strm);
    return Z_ERRNO;
    }
    if (strm.avail_in == 0)
    break;
    strm.next_in = inBuff;

    /* run inflate() on input until output buffer not full */
    do {
    strm.avail_out = CHUNK;
    strm.next_out = outBuff;
    ret = inflate(&strm, Z_NO_FLUSH);
    assert(ret != Z_STREAM_ERROR);  /* state not clobbered */

    if (ret == Z_NEED_DICT)
    {
        ret = Z_DATA_ERROR;
    }
    if (ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
    {
        (void)inflateEnd(&strm);
        return ret;
    }

    have = CHUNK - strm.avail_out;

    data.insert(data.end(), outBuff, outBuff + have);
    /*if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
        (void)inflateEnd(&strm);
        return Z_ERRNO;
    }*/
    } while (strm.avail_out == 0);

    /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

int inf(const std::vector<uint8_t>& inData, std::vector<uint8_t>& outData)
{
    const int CHUNK = 16384;
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char outBuff[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    //ret = inflateInit(&strm); // deflate
    ret = inflateInit2(&strm, 16+MAX_WBITS); // gzip
    if (ret != Z_OK)
    return ret;

    /* decompress until deflate stream ends or end of file */
    {
        strm.avail_in = inData.size();
        strm.next_in = inData.data();

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = outBuff;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */

            if (ret == Z_NEED_DICT)
            {
                ret = Z_DATA_ERROR;
            }
            if (ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
            {
                (void)inflateEnd(&strm);
                return ret;
            }

            have = CHUNK - strm.avail_out;

            outData.insert(outData.end(), outBuff, outBuff + have);
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
        if (ret != Z_STREAM_END)
        {
            ret = Z_DATA_ERROR;
        }
    }

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

int loadMeshZ(const std::vector<uint8_t>& inData, vtslibs::vts::Mesh& mesh)
{
    const int ok = 0;
    const int err = 1;

    std::vector<uint8_t> data;

    if (inf(inData, data) != Z_OK)
    {
        return err;
    }

    membuf sbuf((char*)data.data(), (char*)data.data() + data.size());
    std::istream in(&sbuf);

    vtslibs::vts::detail::loadMeshProper(in, mesh);
    return ok;
}

int loadFile(const char* fname, std::vector<uint8_t>& data)
{
    const int ok = 0;
    const int err = 1;
    data.clear();

    FILE* fin = fopen(fname, "rb");
    if (!fin)
    {
        return err;
    }

    fseek(fin, 0, SEEK_END);
    long sz = ftell(fin);
    rewind(fin);

    data.resize(sz);

    if (fread(data.data(), 1, data.size(), fin) != data.size())
    {
        fclose(fin);
        return err;
    }

    fclose(fin);
    return ok;
}

int loadMeshZ(const char* fname, vtslibs::vts::Mesh& mesh)
{
    const int ok = 0;
    const int err = 1;

    std::vector<uint8_t> inData;

    if (loadFile(fname, inData) != ok)
    {
        return err;
    }

    std::vector<uint8_t> data;

    if (inf(inData, data) != ok)
    {
        return err;
    }

    FILE* fin = fopen(fname, "rb");
    inf(fin, data);
    fclose(fin);

    /* from memory */
    membuf sbuf((char*)data.data(), (char*)data.data() + data.size());
    std::istream in(&sbuf);

    vtslibs::vts::detail::loadMeshProper(in, mesh);
    return ok;
}

int loadMeshZ2(const char* fname, vtslibs::vts::Mesh& mesh)
{
    const int ok = 0;
    //const int err = 1;

    /*gzFile infile = gzopen(fname, "rb");
    if (!infile) return err;*/

    std::vector<uint8_t> data;

    FILE* fin = fopen(fname, "rb");
    inf(fin, data);
    fclose(fin);


    /*char buffer[1024];
    int num_read = 0;
    while ((num_read = gzread(infile, buffer, sizeof(buffer))) > 0) {
    //fwrite(buffer, 1, num_read, outfile);
    data.insert(data.end(), buffer, buffer + sizeof(buffer));
    }

    gzclose(infile);*/

    //static std::istream *currentReadStream;

    /* from memory.. */

    membuf sbuf((char*)data.data(), (char*)data.data() + data.size());
    std::istream in(&sbuf);
    //currentReadStream = &in;

    /* or from file */
    /*std::ifstream is(filename, ios::binary);
    currentReadStream = &file;*/



    vtslibs::vts::detail::loadMeshProper(in, mesh);
    return ok;
}

int loadMeshZ1(const char* fname, vtslibs::vts::Mesh& mesh)
{
    const int ok = 0;
    const int err = 1;

    gzFile infile = gzopen(fname, "rb");
    if (!infile) return err;

    std::vector<uint8_t> data;

    char buffer[1024];
    int num_read = 0;
    while ((num_read = gzread(infile, buffer, sizeof(buffer))) > 0) {
        //fwrite(buffer, 1, num_read, outfile);
        data.insert(data.end(), buffer, buffer + sizeof(buffer));
    }

    gzclose(infile);

    //static std::istream *currentReadStream;

    /* from memory.. */

    membuf sbuf((char*)data.data(), (char*)data.data() + data.size());
    std::istream in(&sbuf);
    //currentReadStream = &in;

    /* or from file */
    /*std::ifstream is(filename, ios::binary);
    currentReadStream = &file;*/



    vtslibs::vts::detail::loadMeshProper(in, mesh);
    return ok;
}
