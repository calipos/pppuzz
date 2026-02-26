#include "random.h"
int ThreadSafeRandom::randInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_engine);
}
double ThreadSafeRandom::randDouble(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(m_engine);
}
ThreadSafeRandom::ThreadSafeRandom() {
    std::random_device rd;
    auto seed = rd() ^ std::chrono::steady_clock::now().time_since_epoch().count()
        ^ std::hash<std::thread::id>{}(std::this_thread::get_id()); // 混合线程ID，增加种子差异
    m_engine.seed(seed);
}
 
ThreadSafeRandom& ThreadSafeRandom::getInstance() {
    static thread_local ThreadSafeRandom instance;
    return instance;
}

  