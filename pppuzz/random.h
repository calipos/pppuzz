#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <random>
#include <thread>
#include <iostream>

class ThreadSafeRandom {
public:
    static ThreadSafeRandom& getInstance();
    int randInt(int min, int max);
    double randDouble(double min, double max);
private:
    ThreadSafeRandom();
    std::mt19937 m_engine;
};

#endif // !__RANDOM_H__
