#pragma once

#include "ThreadPool.h"

namespace v4d::tests 
{
    void ThreadPool() 
    {
        //TODO replace this with a proper test

        {
            v4d::processing::ThreadPool threadPool(2);

            auto future = threadPool.promise([]{
                SLEEP(5s)
                return 154;
            });

            LOG("Calculating...")

            threadPool.enqueue([]{
                SLEEP(10s)
            });

            int res = future.get();
            LOG(res)
        }

        LOG("Finished!")
        
    }
}
