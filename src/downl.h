#ifndef DOWNL_H
#define DOWNL_H

#include <vector>
#include <cstdint>

int downlGet(const char* url, std::vector<uint8_t>& data);

#endif // DOWNL_H
