#include "pch.h"
#include "CacheManager.h"
#include "Globals.h"
#include "CheatFunction.h"
#include "PlayerEsp.h"
#include "WorldEntity.h"
#include "ConfigUtilities.h"
#include <algorithm>

namespace {
bool CanRun(Environment *env) {
    return env != nullptr && env->GetObjectCount() > 10;
}

struct CacheTimingProfile {
    int globalsSleepMs;
    int entityRebuildPauseMs;
    int entityIdleSleepMs;
    int entityNoObjectsSleepMs;
    int delayedChangeRefreshSec;
    int periodicRefreshSec;
    int transformSleepMs;
    int boneSleepMs;
};

CacheTimingProfile GetCacheTimingProfile()
{
    switch (std::clamp(Configs.General.CacheLevel, 0, 3))
    {
    case 0: // Low: smaller caches, faster refresh cadence.
        return { 350, 650, 700, 900, 30, 180, 28, 20 };
    case 2: // High: larger caches, slightly slower refresh.
        return { 700, 1100, 1200, 1400, 60, 420, 55, 38 };
    case 3: // Very high: maximum cache window, conservative refresh.
        return { 900, 1300, 1500, 1700, 75, 600, 70, 50 };
    case 1:
    default: // Default profile (current behavior baseline).
        return { 550, 900, 1000, 1200, 45, 300, 45, 30 };
    }
}
}

// Initialize global instance
CacheManager g_CacheManager;

void CacheManager::GlobalsThreadFunction()
{
    while (should_run)
    {
        const CacheTimingProfile profile = GetCacheTimingProfile();
        if (EnvironmentInstance)
        {
            EnvironmentInstance->GetEntities();
            if (CanRun(EnvironmentInstance.get()))
                EnvironmentInstance->UpdateLocalPlayer();
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(profile.globalsSleepMs));
    }
}

void CacheManager::EntityArrayThreadFunction()
{
    uint16_t lastObjectCount = 0;
    auto lastFullCache = std::chrono::steady_clock::now() - std::chrono::seconds(30);

    while (should_run)
    {
        const CacheTimingProfile profile = GetCacheTimingProfile();
        if (CanRun(EnvironmentInstance.get()))
        {
            const uint16_t objectCount = EnvironmentInstance->GetObjectCount();
            const bool firstRun = (lastObjectCount == 0);
            const bool objectCountChanged = (objectCount != lastObjectCount);
            const int objectCountDelta =
                std::abs(static_cast<int>(objectCount) - static_cast<int>(lastObjectCount));
            const bool significantCountChange = objectCountDelta >= 256;
            const bool delayedChangeRefresh =
                objectCountChanged &&
                (std::chrono::steady_clock::now() - lastFullCache) >=
                    std::chrono::seconds(profile.delayedChangeRefreshSec);
            const bool periodicRefresh =
                (std::chrono::steady_clock::now() - lastFullCache) >=
                std::chrono::seconds(profile.periodicRefreshSec);

            if (firstRun || significantCountChange || delayedChangeRefresh || periodicRefresh)
            {
                EnvironmentInstance->CacheEntities();
                lastFullCache = std::chrono::steady_clock::now();
                lastObjectCount = objectCount;
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(profile.entityRebuildPauseMs));
                continue;
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds(profile.entityIdleSleepMs));
            continue;
        }

        lastObjectCount = 0;
        std::this_thread::sleep_for(
            std::chrono::milliseconds(profile.entityNoObjectsSleepMs));
    }
}

void CacheManager::TransformThreadFunction()
{
    // Reads HP, render node, extraction flags, spectator count.
    // Positions are intentionally NOT read here — they are read on the
    // render thread (Main.cpp) together with the camera so they are always
    // from the same DMA moment (eliminates ESP drift).
    while (should_run)
    {
        const CacheTimingProfile profile = GetCacheTimingProfile();
        if (CanRun(EnvironmentInstance.get()))
        {
            UpdatePlayers->Execute();
            UpdateBosses->Execute();
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(profile.transformSleepMs));
    }
}

void CacheManager::BoneThreadFunction()
{
    // Dedicated bone cache thread — runs at ~30ms intervals (≈33 Hz).
    // Updates bones for all visible enemy players so that ESP and Aimbot
    // can read pre-cached bone positions without triggering DMA reads
    // on the hot render/aimbot paths.
    while (should_run)
    {
        const CacheTimingProfile profile = GetCacheTimingProfile();
        if (CanRun(EnvironmentInstance.get()))
        {
            const auto& templist = EnvironmentInstance->GetPlayerList();
            for (const auto& ent : templist)
            {
                if (!ent) continue;
                if (ent->GetType() != EntityType::EnemyPlayer) continue;
                if (!ent->GetValid() || ent->IsHidden()) continue;
                ent->UpdateBones();
            }
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(profile.boneSleepMs));
    }
}

void CacheManager::Start()
{
    if (should_run.exchange(true))
        return;

    if (!globals_thread) {
        globals_thread = std::make_unique<std::thread>(
            &CacheManager::GlobalsThreadFunction, this);
    }
    if (!entity_array_thread) {
        entity_array_thread = std::make_unique<std::thread>(
            &CacheManager::EntityArrayThreadFunction, this);
    }
    if (!transform_thread) {
        transform_thread = std::make_unique<std::thread>(
            &CacheManager::TransformThreadFunction, this);
    }
    if (!bone_thread) {
        bone_thread = std::make_unique<std::thread>(
            &CacheManager::BoneThreadFunction, this);
    }

    LOG_INFO("Cache manager threads started");
}

void CacheManager::Stop()
{
    if (!should_run.exchange(false))
        return;

    if (globals_thread && globals_thread->joinable())
        globals_thread->join();
    if (entity_array_thread && entity_array_thread->joinable())
        entity_array_thread->join();
    if (transform_thread && transform_thread->joinable())
        transform_thread->join();
    if (bone_thread && bone_thread->joinable())
        bone_thread->join();

    globals_thread.reset();
    entity_array_thread.reset();
    transform_thread.reset();
    bone_thread.reset();

    LOG_INFO("Cache manager threads stopped");
}

CacheManager::~CacheManager()
{
    Stop();
}