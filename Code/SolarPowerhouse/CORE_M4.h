#ifndef CORE_M4_H
#define CORE_M4_H

#include "sharedMemory.h"
#include "thingProperties.h"
#include "Util.h"

class CORE_M4 {
    private:
    sharedMemory ipc;

    public: 

    auto getVarsFromSharedMemory() -> void;
};

#endif // CORE_M4