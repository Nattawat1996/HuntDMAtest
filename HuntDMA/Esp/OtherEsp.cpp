#include "CheatFunction.h"
#include "ConfigInstance.h"
#include "ConfigUtilities.h"
#include "ESPRenderer.h"
#include "Localization/Localization.h"
#include "Pch.h"
#include "PlayerEsp.h"
#include "globals.h"
#include <WorldEntity.h>

void DrawSupply() {
  const auto &templist = EnvironmentInstance->GetSupplyList();

  if (templist.empty())
    return;

  for (std::shared_ptr<WorldEntity> ent : templist) {
    if (ent == nullptr)
      continue;

    auto type = ent->GetType();
    if (!Configs.Supply.ShowAmmoSwapBox &&
        (type == EntityType::AmmoSwapBox ||
         type == EntityType::SpitzerBullets ||
         type == EntityType::PoisonBullets ||
         type == EntityType::HighVelocityBullets ||
         type == EntityType::IncendiaryBullets ||
         type == EntityType::DumDumBullets ||
         type == EntityType::ExplosiveBullets ||
         type == EntityType::FMJBullets || type == EntityType::PoisonBolts ||
         type == EntityType::ExplosiveBolts ||
         type == EntityType::IncendiaryShells ||
         type == EntityType::PennyShotShells ||
         type == EntityType::FlechetteShells || type == EntityType::SlugShells))
      continue;
    if (!Configs.Supply.ShowCompactAmmo &&
        (type == EntityType::CompactAmmoPackage))
      continue;
    if (!Configs.Supply.ShowMediumAmmo &&
        (type == EntityType::MediumAmmoPackage))
      continue;
    if (!Configs.Supply.ShowLongAmmo && (type == EntityType::LongAmmoPackage))
      continue;
    if (!Configs.Supply.ShowShortgunAmmo &&
        (type == EntityType::ShotgunAmmoPackage))
      continue;
    if (!Configs.Supply.ShowSpecialAmmo &&
        (type == EntityType::SpecialAmmoCrate))
      continue;
    if ((!Configs.Supply.ShowCompactAmmo && !Configs.Supply.ShowMediumAmmo &&
         !Configs.Supply.ShowLongAmmo && !Configs.Supply.ShowShortgunAmmo) &&
        (type == EntityType::AmmoCrate))
      continue;
    if (!Configs.Supply.ShowMedkit && (type == EntityType::Medkit))
      continue;
    if (!Configs.Supply.ShowSupplyBox && (type == EntityType::SupplyBox))
      continue;

    int distance = (int)Vector3::Distance(ent->GetPosition(),
                                          CameraInstance->GetPosition());
    if (distance <= 0 || distance > Configs.Supply.MaxDistance)
      continue;

    if (!ent->GetValid())
      continue;
    Vector2 pos = CameraInstance->WorldToScreen(ent->GetPosition());
    if (pos.x == 0 || pos.y == 0)
      continue;

    std::string name =
        Configs.Supply.Name ? LOC("entity", ent->GetTypeAsString()) : "";
    std::string distanceText =
        Configs.Supply.Distance ? std::vformat(LOC("menu", "esp.Meters"),
                                               std::make_format_args(distance))
                                : "";

    ESPRenderer::DrawText(ImVec2(pos.x, pos.y), name + distanceText,
                          Configs.Supply.TextColor, Configs.Supply.FontSize,
                          Center);
  }
}

void DrawBloodBonds() {
  const auto &templist = EnvironmentInstance->GetBloodBondsList();

  if (templist.empty())
    return;

  for (std::shared_ptr<WorldEntity> ent : templist) {
    if (ent == nullptr)
      continue;

    int distance = (int)Vector3::Distance(ent->GetPosition(),
                                          CameraInstance->GetPosition());
    if (distance <= 0 || distance > Configs.BloodBonds.MaxDistance)
      continue;

    if (!ent->GetValid())
      continue;
    Vector2 pos = CameraInstance->WorldToScreen(ent->GetPosition());
    if (pos.x == 0 || pos.y == 0)
      continue;

    std::string name =
        Configs.BloodBonds.Name ? LOC("entity", ent->GetTypeAsString()) : "";
    std::string distanceText =
        Configs.BloodBonds.Distance
            ? std::vformat(LOC("menu", "esp.Meters"),
                           std::make_format_args(distance))
            : "";

    ESPRenderer::DrawText(ImVec2(pos.x, pos.y), name + distanceText,
                          Configs.BloodBonds.TextColor,
                          Configs.BloodBonds.FontSize, Center);
  }
}

void DrawTraps() {
  const auto &templist = EnvironmentInstance->GetTrapList();

  if (templist.empty())
    return;

  for (std::shared_ptr<WorldEntity> ent : templist) {
    if (ent == nullptr)
      continue;

    auto type = ent->GetType();
    if (!Configs.Trap.ShowBeartrap && (type == EntityType::BearTrap))
      continue;
    if (!Configs.Trap.ShowTripmines && (type == EntityType::TripMine))
      continue;
    if (!Configs.Trap.ShowDarksightDynamite &&
        (type == EntityType::DarksightDynamite))
      continue;
    if (!Configs.Trap.ShowGunpowderBurrels &&
        (type == EntityType::GunpowderBarrel))
      continue;
    if (!Configs.Trap.ShowOilBurrels && (type == EntityType::OilBarrel))
      continue;
    if (!Configs.Trap.ShowBioBurrels && (type == EntityType::BioBarrel))
      continue;

    int distance = (int)Vector3::Distance(ent->GetPosition(),
                                          CameraInstance->GetPosition());
    if (distance <= 0 || distance > Configs.Trap.MaxDistance)
      continue;

    if (!ent->GetValid())
      continue;
    Vector2 pos = CameraInstance->WorldToScreen(ent->GetPosition());
    if (pos.x == 0 || pos.y == 0)
      continue;

    std::string name =
        Configs.Trap.Name ? LOC("entity", ent->GetTypeAsString()) : "";
    std::string distanceText =
        Configs.Trap.Distance ? std::vformat(LOC("menu", "esp.Meters"),
                                             std::make_format_args(distance))
                              : "";

    ESPRenderer::DrawText(ImVec2(pos.x, pos.y), name + distanceText,
                          (type == EntityType::BearTrap ||
                           type == EntityType::TripMine ||
                           type == EntityType::DarksightDynamite)
                              ? Configs.Trap.TrapColor
                              : Configs.Trap.BarrelColor,
                          Configs.Trap.FontSize, Center);
  }
}

void DrawPOI() {
  const auto &templist = EnvironmentInstance->GetPOIList();

  if (templist.empty())
    return;

  for (std::shared_ptr<WorldEntity> ent : templist) {
    if (ent == nullptr)
      continue;

    auto type = ent->GetType();
    if (!Configs.POI.ShowResupplyStation &&
        (type == EntityType::ResupplyStation))
      continue;
    if (!Configs.POI.ShowExtraction && (type == EntityType::ExtractionPoint))
      continue;
    if (!Configs.POI.ShowCashRegisters && (type == EntityType::CashRegister))
      continue;
    if (!Configs.POI.ShowPouches && (type == EntityType::Pouch))
      continue;
    if (!Configs.POI.ShowPosters && (type == EntityType::Poster))
      continue;
    if (!Configs.POI.ShowBlueprints && (type == EntityType::WorkbenchUpgrade))
      continue;
    if (!Configs.POI.ShowGunOil && (type == EntityType::WorkbenchOil))
      continue;
    if (!Configs.POI.ShowClues && (type == EntityType::Clue))
      continue;
    if (!Configs.POI.ShowSeasonalDestructibles && (type == EntityType::Event))
      continue;

    int distance = (int)Vector3::Distance(ent->GetPosition(),
                                          CameraInstance->GetPosition());
    if (distance <= 0 || distance > Configs.POI.MaxDistance)
      continue;

    if (!Configs.POI.ShowBoons && (type == EntityType::EventBoon))
      continue;

    if (!ent->GetValid())
      continue;

    Vector2 pos = CameraInstance->WorldToScreen(ent->GetPosition());
    if (pos.x == 0 || pos.y == 0)
      continue;

    std::string name =
        Configs.POI.Name ? LOC("entity", ent->GetTypeAsString()) : "";
    if (type == EntityType::EventBoon && !ent->CompactTypeName.empty())
      name = ent->CompactTypeName;

    std::string distanceText =
        Configs.POI.Distance ? std::vformat(LOC("menu", "esp.Meters"),
                                            std::make_format_args(distance))
                             : "";

    ImVec4 color = Configs.POI.TextColor;
    if (type == EntityType::EventBoon)
      color = Configs.POI.BoonColor;

    ESPRenderer::DrawText(ImVec2(pos.x, pos.y), name + distanceText, color,
                          Configs.POI.FontSize, Center);
  }
}

void DrawTraits() {
  const auto &templist = EnvironmentInstance->GetTraitList();

  if (templist.empty())
    return;

  for (std::shared_ptr<WorldEntity> ent : templist) {
    if (ent == nullptr)
      continue;

    auto compactTypeName = ent->CompactTypeName;

    int distance = (int)Vector3::Distance(ent->GetPosition(),
                                          CameraInstance->GetPosition());
    if (distance <= 0)
      continue;

    auto color = Configs.Traits.TraitColor;

    if (compactTypeName == "beastface") {
      if (!Configs.Traits.EnableBeastface)
        continue;
      if (distance > Configs.Traits.BeastfaceDistance)
        continue;
    } else if (compactTypeName == "berserker") {
      if (!Configs.Traits.EnableBerserker)
        continue;
      if (distance > Configs.Traits.BerserkerDistance)
        continue;
    } else if (compactTypeName == "bloodless") {
      if (!Configs.Traits.EnableBloodless)
        continue;
      if (distance > Configs.Traits.BloodlessDistance)
        continue;
    } else if (compactTypeName == "bulletgrubber") {
      if (!Configs.Traits.EnableBulletgrubber)
        continue;
      if (distance > Configs.Traits.BulletgrubberDistance)
        continue;
    } else if (compactTypeName == "conduit") {
      if (!Configs.Traits.EnableConduit)
        continue;
      if (distance > Configs.Traits.ConduitDistance)
        continue;
    } else if (compactTypeName == "deathcheat") {
      if (!Configs.Traits.EnableDeathcheat)
        continue;
      if (distance > Configs.Traits.DeathcheatDistance)
        continue;
      color = Configs.BloodBonds.TextColor;
    } else if (compactTypeName == "determination") {
      if (!Configs.Traits.EnableDetermination)
        continue;
      if (distance > Configs.Traits.DeterminationDistance)
        continue;
    } else if (compactTypeName == "doctor") {
      if (!Configs.Traits.EnableDoctor)
        continue;
      if (distance > Configs.Traits.DoctorDistance)
        continue;
    } else if (compactTypeName == "fanning") {
      if (!Configs.Traits.EnableFanning)
        continue;
      if (distance > Configs.Traits.FanningDistance)
        continue;
    } else if (compactTypeName == "fastfingers") {
      if (!Configs.Traits.EnableFastfingers)
        continue;
      if (distance > Configs.Traits.FastfingersDistance)
        continue;
    } else if (compactTypeName == "gatorlegs") {
      if (!Configs.Traits.EnableGatorlegs)
        continue;
      if (distance > Configs.Traits.GatorlegsDistance)
        continue;
    } else if (compactTypeName == "ghoul") {
      if (!Configs.Traits.EnableGhoul)
        continue;
      if (distance > Configs.Traits.GhoulDistance)
        continue;
    } else if (compactTypeName == "greyhound") {
      if (!Configs.Traits.EnableGreyhound)
        continue;
      if (distance > Configs.Traits.GreyhoundDistance)
        continue;
    } else if (compactTypeName == "levering") {
      if (!Configs.Traits.EnableLevering)
        continue;
      if (distance > Configs.Traits.LeveringDistance)
        continue;
    } else if (compactTypeName == "lightfoot") {
      if (!Configs.Traits.EnableLightfoot)
        continue;
      if (distance > Configs.Traits.LightfootDistance)
        continue;
    } else if (compactTypeName == "magpie") {
      if (!Configs.Traits.EnableMagpie)
        continue;
      if (distance > Configs.Traits.MagpieDistance)
        continue;
    } else if (compactTypeName == "necromancer") {
      if (!Configs.Traits.EnableNecromancer)
        continue;
      if (distance > Configs.Traits.NecromancerDistance)
        continue;
    } else if (compactTypeName == "packmule") {
      if (!Configs.Traits.EnablePackmule)
        continue;
      if (distance > Configs.Traits.PackmuleDistance)
        continue;
    } else if (compactTypeName == "physician") {
      if (!Configs.Traits.EnablePhysician)
        continue;
      if (distance > Configs.Traits.PhysicianDistance)
        continue;
    } else if (compactTypeName == "pitcher") {
      if (!Configs.Traits.EnablePitcher)
        continue;
      if (distance > Configs.Traits.PitcherDistance)
        continue;
    } else if (compactTypeName == "quartermaster") {
      if (!Configs.Traits.EnableQuartermaster)
        continue;
      if (distance > Configs.Traits.QuartermasterDistance)
        continue;
    } else if (compactTypeName == "rampage") {
      if (!Configs.Traits.EnableRampage)
        continue;
      if (distance > Configs.Traits.RampageDistance)
        continue;
    } else if (compactTypeName == "relentless") {
      if (!Configs.Traits.EnableRelentless)
        continue;
      if (distance > Configs.Traits.RelentlessDistance)
        continue;
    } else if (compactTypeName == "remedy") {
      if (!Configs.Traits.EnableRemedy)
        continue;
      if (distance > Configs.Traits.RemedyDistance)
        continue;
    } else if (compactTypeName == "resilience") {
      if (!Configs.Traits.EnableResilience)
        continue;
      if (distance > Configs.Traits.ResilienceDistance)
        continue;
    } else if (compactTypeName == "salveskin") {
      if (!Configs.Traits.EnableSalveskin)
        continue;
      if (distance > Configs.Traits.SalveskinDistance)
        continue;
    } else if (compactTypeName == "serpent") {
      if (!Configs.Traits.EnableSerpent)
        continue;
      if (distance > Configs.Traits.SerpentDistance)
        continue;
    } else if (compactTypeName == "shadow") {
      if (!Configs.Traits.EnableShadow)
        continue;
      if (distance > Configs.Traits.ShadowDistance)
        continue;
    } else if (compactTypeName == "shadowleap") {
      if (!Configs.Traits.EnableShadowleap)
        continue;
      if (distance > Configs.Traits.ShadowleapDistance)
        continue;
    } else if (compactTypeName == "vigor") {
      if (!Configs.Traits.EnableVigor)
        continue;
      if (distance > Configs.Traits.VigorDistance)
        continue;
    } else if (compactTypeName == "whispersmith") {
      if (!Configs.Traits.EnableWhispersmith)
        continue;
      if (distance > Configs.Traits.WhispersmithDistance)
        continue;
    } else if (compactTypeName == "witness") {
      if (!Configs.Traits.EnableWitness)
        continue;
      if (distance > Configs.Traits.WitnessDistance)
        continue;
    } else if (compactTypeName == "blademancer") {
      if (!Configs.Traits.EnableBlademancer)
        continue;
      if (distance > Configs.Traits.BlademancerDistance)
        continue;
    } else if (compactTypeName == "corpseseer") {
      if (!Configs.Traits.EnableCorpseseer)
        continue;
      if (distance > Configs.Traits.CorpseseerDistance)
        continue;
    } else if (compactTypeName == "gunrunner") {
      if (!Configs.Traits.EnableGunrunner)
        continue;
      if (distance > Configs.Traits.GunrunnerDistance)
        continue;
    } else {
      if (!Configs.Traits.EnableOther)
        continue;
      if (distance > Configs.Traits.OtherDistance)
        continue;
    }

    if (!ent->GetValid())
      continue;

    Vector2 pos = CameraInstance->WorldToScreen(ent->GetPosition());
    if (pos.x == 0 || pos.y == 0)
      continue;

    std::string name = Configs.Traits.Name
                           ? LOC("entity", ent->GetTypeAsString()) + " [" +
                                 LOC("trait", compactTypeName) + "]"
                           : "";
    std::string distanceText =
        Configs.Traits.Distance ? std::vformat(LOC("menu", "esp.Meters"),
                                               std::make_format_args(distance))
                                : "";

    ESPRenderer::DrawText(ImVec2(pos.x, pos.y), name + distanceText, color,
                          Configs.Traits.FontSize, Center);
  }
}

void DrawOtherEsp() {
  if (EnvironmentInstance == nullptr)
    return;

  if (EnvironmentInstance->GetObjectCount() < 10)
    return;

  if (Configs.Supply.Enable)
    DrawSupply();

  if (Configs.BloodBonds.Enable)
    DrawBloodBonds();

  if (Configs.Trap.Enable)
    DrawTraps();

  if (Configs.POI.Enable)
    DrawPOI();

  if (Configs.Traits.Enable)
    DrawTraits();
}