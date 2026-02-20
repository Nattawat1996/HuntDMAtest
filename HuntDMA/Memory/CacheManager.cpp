#include "pch.h"
#include "CacheManager.h"
#include "Globals.h"
#include "CheatFunction.h"

void UpdateLocalPlayer()
{
    if (EnvironmentInstance == nullptr)
        return;
    if (EnvironmentInstance->GetObjectCount() < 10)
        return;
    EnvironmentInstance->UpdateLocalPlayer();
}

void Cache()
{
    if (EnvironmentInstance == nullptr)
        return;
    if (EnvironmentInstance->GetObjectCount() < 10)
        return;
    EnvironmentInstance->CacheEntities();
}

// Initialize global instance
CacheManager g_CacheManager;

void CacheManager::CacheThreadFunction()
{
    while (should_run)
    {
        if (EnvironmentInstance && EnvironmentInstance->GetObjectCount() > 10)
        {
            UpdateLocalPlayer();
            Cache();

            auto sleep_time = std::chrono::milliseconds(8000);

            std::this_thread::sleep_for(sleep_time);
        }
        else {
            // If no work, sleep a bit to prevent CPU spinning
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void CacheManager::Start()
{
    if (!cache_thread) {
        cache_thread = std::make_unique<std::thread>(&CacheManager::CacheThreadFunction, this);
        LOG_INFO("Cache thread started");
    }
}

void CacheManager::Stop()
{
    should_run = false;
    if (cache_thread && cache_thread->joinable()) {
        cache_thread->join();
        LOG_INFO("Cache thread stopped");
    }
}

CacheManager::~CacheManager()
{
    Stop();
}