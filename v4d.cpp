#include "v4d.h"

#ifdef _V4D_CORE
    extern "C" V4DLIB std::string GET_V4D_CORE_BUILD_VERSION() {
        return V4D_VERSION;
    }
#endif
