#include "downl.h"

#include <curl/curl.h>

#include <cstring>

#include <algorithm>

static size_t downlWriteData(void *ptr, size_t size, size_t nmemb, std::vector<uint8_t>* data)
{
    const uint8_t* ubptr = (uint8_t*)ptr;
    for (size_t i = 0; i != size * nmemb; ++i)
    {
        data->push_back(ubptr[i]);
    }
    return nmemb;
}

static int downlTest404(const std::vector<uint8_t>& data)
{
    const int ok = 0;
    const int err404 = 404;
    if (data.size() < 256)
    {
        static const uint8_t notFoundText[] = "404 Not Found";
        if (std::search(data.begin(), data.end(), notFoundText, notFoundText + strlen((const char*)notFoundText)) != data.end())
        {
            return err404;
        }
    }
    return ok;
}

int downlGet(const char* url, std::vector<uint8_t>& data)
{
    const int err = 1;
    data.clear();
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        return err;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downlWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK)
    {
        return err;
    }
    return downlTest404(data);
}
