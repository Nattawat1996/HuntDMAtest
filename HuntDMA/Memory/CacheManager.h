#pragma once
#include "pch.h"

class CacheManager {
private:
    std::atomic<bool> should_run{ false };
    std::unique_ptr<std::thread> globals_thread;
    std::unique_ptr<std::thread> entity_array_thread;
    std::unique_ptr<std::thread> transform_thread;
    std::unique_ptr<std::thread> bone_thread;  // Dedicated bone cache thread

    void GlobalsThreadFunction();
    void EntityArrayThreadFunction();
    void TransformThreadFunction();
    void BoneThreadFunction();   // Caches bones for all visible enemy players

public:
    CacheManager() = default;
    ~CacheManager();

    void Start();
    void Stop();
};

// Global instance
extern CacheManager g_CacheManager;