#include "synchronized_ostream.hpp"
#include <iostream>
#include <thread>

synchronized_ostream sync_cout(std::cout);

// Used for testing whether synchronized_ostream works or doesnt. 
void test()
{
    const auto numThreads = 30;
    std::thread t[numThreads];

    for (auto i = 0; i < numThreads; ++i)
    {
        t[i] = std::thread([](int threadId) { sync_cout << "Launched by thread " << threadId << std::endl; }, i);
    }

    sync_cout << "Launched from the main thread" << std::endl;

    for (auto i = 0; i < numThreads; ++i) 
    {
        t[i].join();
    }
}