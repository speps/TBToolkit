#include <TBToolkit/Common.h>

namespace TB
{
    uint32_t hash(const uint8_t* data, size_t len)
    {
        uint32_t h = 0;
        for (size_t i = 0; i < len; i++)
        {
            h = h * 65599 + data[i];
        }
        return h;
    }
}
