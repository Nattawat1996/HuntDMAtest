#pragma once
#include "WorldEntity.h"
enum class MapType : int {
  None,
  Unknown,
  StillwaterBayou,
  LawsonDelta,
  LawsonDeltaShootingRange,
  DeSalle,
  MammonsGulch,
};
class Environment {
private:
  uint64_t SystemGlobalEnvironment = 0x0;
  const uint64_t SystemGlobalEnvironmentOffset =
      0x0; // You can manually overwrite offset, if signature find is not
           // working
           // https://www.unknowncheats.me/forum/other-fps-games/350352-hunt-showdown-27.html
           // someone would post it here.
  const char *SystemGlobalEnvironmentSignature = "48 8B 05 ? ? ? ? 48 8B 88 B0";
  void FindSystemGlobalEnvironment();

  uint64_t EntitySystem = 0x0;
  const uint64_t EntitySystemOffset = 0xC0;

  uint64_t pSystem = 0x0;
  const uint64_t pSystemOffset = 0x90;

  uint16_t ObjectCount = 0x0;
  const uint64_t ObjectCountOffset = 0x40092;

  uint64_t EntityList = 0x0;
  const uint64_t EntityListOffset = 0x40098;

  uint64_t IGameFramework = 0x0;
  const uint64_t IGameFrameworkOffset = 0xD8;

  uint64_t pGame = 0x0;
  const uint64_t pGameOffset = 0x700;

  uint64_t pGameClientNub = 0x0;
  const uint64_t pGameClientNubOffset = 0x48;

  uint64_t pGameClientChannel = 0x0;
  const uint64_t pGameClientChannelOffset = 0x20;

  uint64_t localPlayerIdx = 0x0;
  const uint64_t localPlayerIdxOffset = 0x24;

  uint64_t LocalPlayer = 0x0;

  int SpectatorCount = 0;

  std::vector<std::shared_ptr<WorldEntity>> PlayerList;
  std::vector<std::shared_ptr<WorldEntity>> BossesList;
  std::vector<std::shared_ptr<WorldEntity>> SupplyList;
  std::vector<std::shared_ptr<WorldEntity>> BloodBondsList;
  std::vector<std::shared_ptr<WorldEntity>> TrapList;
  std::vector<std::shared_ptr<WorldEntity>> POIList;
  std::vector<std::shared_ptr<WorldEntity>> TraitList;

public:
  uint64_t GetSystemGlobalEnvironment() { return SystemGlobalEnvironment; }
  uint64_t GetEntitySystem() { return EntitySystem; }
  uint64_t GetpSystem() { return pSystem; }
  uint16_t GetObjectCount() { return ObjectCount; }
  uint64_t GetEntityList() { return EntityList; }
  uint64_t GetLocalPlayerPointer() { return LocalPlayer; }
  void UpdateLocalPlayer();
  void LogTrait();
  void GetEntities();
  void UpdatePlayerList();
  void UpdateBossesList();
  void CacheEntities();
  void AssignMapType(char *name);
  Environment();
  void ClearConsole();
  const std::vector<std::shared_ptr<WorldEntity>> &GetPlayerList() {
    return PlayerList;
  }
  const std::vector<std::shared_ptr<WorldEntity>> &GetBossesList() {
    return BossesList;
  }
  const std::vector<std::shared_ptr<WorldEntity>> &GetSupplyList() {
    return SupplyList;
  }
  const std::vector<std::shared_ptr<WorldEntity>> &GetBloodBondsList() {
    return BloodBondsList;
  }
  const std::vector<std::shared_ptr<WorldEntity>> &GetTrapList() {
    return TrapList;
  }
  const std::vector<std::shared_ptr<WorldEntity>> &GetPOIList() {
    return POIList;
  }
  const std::vector<std::shared_ptr<WorldEntity>> &GetTraitList() {
    return TraitList;
  }
  int GetSpectatorCount() { return SpectatorCount; }
  MapType mapType = MapType::None;
};
extern bool createEntitiesDump;