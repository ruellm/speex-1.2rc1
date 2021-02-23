#pragma once

namespace libspeex_adapter {
    int create(int quality);
    int getSamplingRate(int quality);
    int getFrameSize(int instanceId, int quality);
    int compress(int instanceId, int len, char *buf);
    int decompress(int instanceId, int len, char *buf);
    void destroy(int instanceId);
}