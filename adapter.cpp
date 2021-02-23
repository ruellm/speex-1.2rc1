#include <mutex>
#include <map>

#include "speexproj.h"
#include "adapter.h"

static std::map<int, void*> instanceList_;
static int lastId_ = 0;
static std::mutex idMutex_;

namespace libspeex_adapter {

    int create(int quality) {
        auto instance = spexproj::create(quality);

        idMutex_.lock();
        auto id = ++lastId_;
        idMutex_.unlock();

        instanceList_.insert(std::make_pair(id, instance));

        return id;
    }

    int getSamplingRate(int quality) {
        return spexproj::getSamplingRate(quality);
    }

    int getFrameSize(int instanceId, int quality) {
        auto instance = instanceList_.find(instanceId);
        if (instance == instanceList_.end())
            return -1;

        auto context = instance->second;
        return spexproj::getFrameSize(context, quality);
    }

    int compress(int instanceId, int len, char *buf) {
        auto instance = instanceList_.find(instanceId);
        if (instance == instanceList_.end())
            return -1;

        auto context = instance->second;
        return spexproj::compress(context, len, buf);
    }

    int decompress(int instanceId, int len, char *buf) {
        auto instance = instanceList_.find(instanceId);
        if (instance == instanceList_.end())
            return -1;

        auto context = instance->second;
        return spexproj::decompress(context, len, buf);
    }

    void destroy(int instanceId) {
        auto instance = instanceList_.find(instanceId);
        if (instance == instanceList_.end())
            return;

        auto context = instance->second;
        spexproj::destroy(context);
    }
}