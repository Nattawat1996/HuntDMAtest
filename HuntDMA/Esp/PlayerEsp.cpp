#include "PlayerEsp.h"
#include "CheatFunction.h"
#include "ConfigInstance.h"
#include "ConfigUtilities.h"
#include "ESPRenderer.h"
#include "Localization/Localization.h"
#include "Pch.h"
#include "WorldEntity.h"
#include "globals.h"

std::shared_ptr<CheatFunction> UpdatePlayers =
    std::make_shared<CheatFunction>(1, [] {
      EnvironmentInstance->UpdatePlayerList();
      // Update velocity for all players (DMA-rate limited by time guard in
      // UpdateVelocity itself)
      for (auto &player : EnvironmentInstance->GetPlayerList()) {
        if (player)
          player->UpdateVelocity();
      }
    });
std::shared_ptr<CheatFunction> UpdateBosses = std::make_shared<CheatFunction>(
    5, [] { EnvironmentInstance->UpdateBossesList(); });

// Helper function to get health bar color based on HP percentage
static ImVec4 GetHealthBarColor(unsigned int currentHp, unsigned int maxHp) {
  if (maxHp == 0)
    return ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray for invalid

  float percentage = (float)currentHp / (float)maxHp;

  if (percentage > 0.75f) // 100%-75% = Green
    return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
  else if (percentage > 0.50f) // 75%-50% = Yellow
    return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
  else if (percentage > 0.25f) // 50%-25% = Orange
    return ImVec4(1.0f, 0.647f, 0.0f, 1.0f);
  else // 25%-0% = Red
    return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
}

void DrawBossesEsp() {
  if (EnvironmentInstance == nullptr)
    return;

  if (EnvironmentInstance->GetObjectCount() < 10)
    return;

  if (Configs.Bosses.Enable) {
    const auto &templist = EnvironmentInstance->GetBossesList();
    if (!templist.empty()) {
      for (std::shared_ptr<WorldEntity> ent : templist) {
        if (ent == nullptr)
          continue;
        int distance = (int)Vector3::Distance(ent->GetPosition(),
                                              CameraInstance->GetPosition());
        if (distance <= 0 || distance > Configs.Bosses.MaxDistance)
          continue;

        if (!ent->GetValid())
          continue;
        Vector2 pos = CameraInstance->WorldToScreen(ent->GetPosition());
        if (pos.x <= 0 || pos.y <= 0)
          continue;
        std::string wname =
            Configs.Bosses.Name ? LOC("entity", ent->GetTypeAsString()) : "";
        std::string wdistance =
            Configs.Bosses.Distance
                ? std::vformat(LOC("menu", "esp.Meters"),
                               std::make_format_args(distance))
                : "";
        ESPRenderer::DrawText(ImVec2(pos.x, pos.y), wname + wdistance,
                              Configs.Bosses.TextColor, Configs.Bosses.FontSize,
                              Center);
      }
    }
  }
}

void DrawPlayersEsp() {
  if (EnvironmentInstance == nullptr)
    return;

  if (EnvironmentInstance->GetObjectCount() < 10)
    return;

  if (Configs.Player.Enable || Configs.Player.DrawFrames ||
      Configs.Player.Snaplines) {
    const auto &templist = EnvironmentInstance->GetPlayerList();
    if (templist.empty())
      return;

    Vector2 center;
    if (Configs.General.CrosshairLowerPosition)
      center = Vector2(ESPRenderer::GetScreenWidth() * 0.5f,
                       ESPRenderer::GetScreenHeight() * 0.6f);
    else
      center = Vector2(ESPRenderer::GetScreenWidth() * 0.5f,
                       ESPRenderer::GetScreenHeight() * 0.5f);

    // 1. Collect positions of confirmed DEAD entities (Hunter_Loot only)
    std::vector<Vector3> deadPositions;
    for (std::shared_ptr<WorldEntity> ent : templist) {
      // Strictly check for Loot Box class name to avoid self-referencing bug
      if (ent != nullptr &&
          strstr(ent->GetEntityClassName().name, "Hunter_Loot")) {
        deadPositions.push_back(ent->GetPosition());
        ent->SetType(
            EntityType::DeadPlayer); // Ensure loot boxes are always Dead
      }
    }

    for (std::shared_ptr<WorldEntity> ent : templist) {
      if (ent == nullptr || ent->GetType() == EntityType::LocalPlayer)
        continue;

      // Skip Hunter_Loot entities (they are used for detection only, not to be
      // drawn)
      if (strstr(ent->GetEntityClassName().name, "Hunter_Loot"))
        continue;

      auto playerPos = ent->GetPosition();

      if (!Configs.Player.DrawFriendsHP && !Configs.Player.DrawBonesFriend &&
          ent->GetType() == EntityType::FriendlyPlayer)
        continue;

      if (!ent->GetValid() || ent->IsHidden()) // Has extracted
        continue;

      // Check if it is a Player Model
      if (strstr(ent->GetEntityClassName().name, "HunterBasic")) {
        // 1. Determine team (Friend/Enemy) based on silhouettes_param
        if (ent->GetRenderNode().silhouettes_param == 0x8CD2FF ||
            ent->GetRenderNode().silhouettes_param == 0x3322eeff) {
          ent->SetType(EntityType::FriendlyPlayer);
        } else {
          ent->SetType(EntityType::EnemyPlayer);
        }

        // 2. Check if dead based on HP (primary method)
        auto health = ent->GetHealth();
        if (health.current_hp == 0 && IsValidHP(health.current_max_hp)) {
          ent->SetType(EntityType::DeadPlayer);
        }
        // 3. Fallback: proximity to loot box (for cases where HP read fails)
        else {
          for (const auto &dPos : deadPositions) {
            if (Vector3::Distance(playerPos, dPos) < 1.0f) // 1 meter tolerance
            {
              ent->SetType(EntityType::DeadPlayer);
              break;
            }
          }
        }
      }

      bool isDead = (ent->GetType() == EntityType::DeadPlayer);

      // HP-based dead detection now active

      // 3. Set Teams logic (Redundant but keeps existing structure safe)
      // (Already handled in reset step above)

      if (!Configs.Player.ShowDead && isDead)
        continue;

      int distance =
          (int)Vector3::Distance(playerPos, CameraInstance->GetPosition());
      if (distance <= 0 || distance > (isDead ? Configs.Player.DeadMaxDistance
                                              : Configs.Player.MaxDistance))
        continue;

      auto tempPos = playerPos;
      Vector2 feetPos = CameraInstance->WorldToScreen(playerPos, false);
      if (feetPos.IsZero())
        continue;

      tempPos.z = playerPos.z + 1.7f;
      Vector2 headPos;
      if (Configs.Player.DrawHeadInFrames || Configs.Player.Snaplines) {
        headPos = CameraInstance->WorldToScreen(tempPos, false);
        if (headPos.IsZero())
          continue;
      }

      tempPos.z = playerPos.z + 2.0f;
      Vector2 uppderFramePos;
      if ((Configs.Player.DrawFrames || Configs.Player.DrawHealthBars ||
           Configs.Player.DrawHeadInFrames)) {
        uppderFramePos = CameraInstance->WorldToScreen(tempPos, false);
        if (uppderFramePos.IsZero())
          continue;
      }

      if (Configs.Player.Snaplines && !headPos.IsZero()) {
        auto colour = ent->GetType() == EntityType::FriendlyPlayer
                          ? Configs.Player.FriendColor
                          : Configs.Player.FramesColor;
        ESPRenderer::DrawLine(ImVec2(center.x, center.y),
                              ImVec2(headPos.x, headPos.y), colour, 1.0f);
      }

      tempPos.z = playerPos.z + 2.1f;
      Vector2 healthBarPos;
      if (Configs.Player.DrawHealthBars) {
        healthBarPos = CameraInstance->WorldToScreen(tempPos, false);
        if (healthBarPos.IsZero())
          continue;
      }

      Vector2 offset, normal;
      if ((Configs.Player.DrawFrames || Configs.Player.DrawHealthBars ||
           Configs.Player.DrawHeadInFrames)) {
        Vector2 v = uppderFramePos - feetPos;
        float segmentLength = Vector2::Length(v);
        normal = Vector2(-v.y, v.x);
        offset = normal / (2.0f * 2);
      }

      if (Configs.Player.DrawFrames &&
          ent->GetType() != EntityType::FriendlyPlayer && !isDead) {
        // Determine top position based on state (Alive = 2.0m, Dead/Loot
        // = 1.85m)
        Vector2 topPos = uppderFramePos;
        if (isDead) {
          auto tempPosTop = playerPos;
          tempPosTop.z = playerPos.z + 1.85f;
          topPos = CameraInstance->WorldToScreen(tempPosTop, false);
        }

        if (!topPos.IsZero()) {
          Vector2 v = topPos - feetPos;
          float segmentLength = Vector2::Length(v);
          Vector2 normal = Vector2(-v.y, v.x);
          Vector2 offset = normal / (2.0f * 2);

          Vector2 A1 = feetPos + offset; // Bottom-left
          Vector2 A2 = feetPos - offset; // Bottom-right
          Vector2 B1 = topPos + offset;  // Top-left
          Vector2 B2 = topPos - offset;  // Top-right

          auto colour = ent->GetType() == EntityType::FriendlyPlayer
                            ? Configs.Player.FriendColor
                            : Configs.Player.FramesColor;

          // Corner bracket length (20% of box dimensions)
          // float cornerLength = segmentLength * 0.2f; // Unused variable
          // warning fix float cornerWidth = Vector2::Distance(A1, A2) * 0.2f;

          // Bottom-left corner (L-shape)
          Vector2 BL_horizontal =
              Vector2(A1.x + (A2.x - A1.x) * 0.2f, A1.y + (A2.y - A1.y) * 0.2f);
          Vector2 BL_vertical =
              Vector2(A1.x + (B1.x - A1.x) * 0.2f, A1.y + (B1.y - A1.y) * 0.2f);
          ESPRenderer::DrawLine(ImVec2(A1.x, A1.y),
                                ImVec2(BL_horizontal.x, BL_horizontal.y),
                                colour, 1.5f);
          ESPRenderer::DrawLine(ImVec2(A1.x, A1.y),
                                ImVec2(BL_vertical.x, BL_vertical.y), colour,
                                1.5f);

          // Bottom-right corner
          Vector2 BR_horizontal =
              Vector2(A2.x + (A1.x - A2.x) * 0.2f, A2.y + (A1.y - A2.y) * 0.2f);
          Vector2 BR_vertical =
              Vector2(A2.x + (B2.x - A2.x) * 0.2f, A2.y + (B2.y - A2.y) * 0.2f);
          ESPRenderer::DrawLine(ImVec2(A2.x, A2.y),
                                ImVec2(BR_horizontal.x, BR_horizontal.y),
                                colour, 1.5f);
          ESPRenderer::DrawLine(ImVec2(A2.x, A2.y),
                                ImVec2(BR_vertical.x, BR_vertical.y), colour,
                                1.5f);

          // Top-left corner
          Vector2 TL_horizontal =
              Vector2(B1.x + (B2.x - B1.x) * 0.2f, B1.y + (B2.y - B1.y) * 0.2f);
          Vector2 TL_vertical =
              Vector2(B1.x + (A1.x - B1.x) * 0.2f, B1.y + (A1.y - B1.y) * 0.2f);
          ESPRenderer::DrawLine(ImVec2(B1.x, B1.y),
                                ImVec2(TL_horizontal.x, TL_horizontal.y),
                                colour, 1.5f);
          ESPRenderer::DrawLine(ImVec2(B1.x, B1.y),
                                ImVec2(TL_vertical.x, TL_vertical.y), colour,
                                1.5f);

          // Top-right corner
          Vector2 TR_horizontal =
              Vector2(B2.x + (B1.x - B2.x) * 0.2f, B2.y + (B1.y - B2.y) * 0.2f);
          Vector2 TR_vertical =
              Vector2(B2.x + (A2.x - B2.x) * 0.2f, B2.y + (A2.y - B2.y) * 0.2f);
          ESPRenderer::DrawLine(ImVec2(B2.x, B2.y),
                                ImVec2(TR_horizontal.x, TR_horizontal.y),
                                colour, 1.5f);
          ESPRenderer::DrawLine(ImVec2(B2.x, B2.y),
                                ImVec2(TR_vertical.x, TR_vertical.y), colour,
                                1.5f);

          if (Configs.Player.DrawHeadInFrames) {
            // Calculate radius based on the frame width
            float headRadius =
                Vector2::Distance(headPos + offset, headPos - offset) /
                Configs.Player.HeadCircleSize;

            // Apply offsets to head position
            Vector2 adjustedHeadPos =
                Vector2(headPos.x + Configs.Player.HeadCircleOffsetX,
                        headPos.y + Configs.Player.HeadCircleOffsetY);

            // Draw circle around head
            ESPRenderer::DrawCircle(
                ImVec2(adjustedHeadPos.x, adjustedHeadPos.y), headRadius,
                colour, 1.5f, false);
          }
        }
      }

      if (Configs.Player.DrawHealthBars) {
        auto health = ent->GetHealth();
        Vector2 Health1 = healthBarPos - offset;
        Vector2 Health2 = healthBarPos + offset;
        auto lineHeight =
            std::max(2.0f, Vector2::Distance(Health1, Health2) / 10.0f);
        float currentHp = health.current_hp / 150.0f;
        float currentMaxHp = health.current_max_hp / 150.0f;
        float potentialMaxHp = health.regenerable_max_hp / 150.0f;
        Vector2 currentHpPos =
            Vector2(currentHp * Health2.x + (1 - currentHp) * Health1.x,
                    currentHp * Health2.y + (1 - currentHp) * Health1.y);
        Vector2 currentMaxHpPos =
            Vector2(currentMaxHp * Health2.x + (1 - currentMaxHp) * Health1.x,
                    currentMaxHp * Health2.y + (1 - currentMaxHp) * Health1.y);
        Vector2 potentialMaxHpPos = Vector2(
            potentialMaxHp * Health2.x + (1 - potentialMaxHp) * Health1.x,
            potentialMaxHp * Health2.y + (1 - potentialMaxHp) * Health1.y);

        // Determine health bar color based on HP percentage
        ImVec4 healthColor;
        if (ent->GetType() == EntityType::FriendlyPlayer) {
          // Friendly players: use blue or dynamic color based on config
          healthColor = ImVec4(0.058823f, 0.407843f, 0.909803f, 1.0f); // Blue
        } else {
          // Enemy players: use dynamic color based on HP percentage
          healthColor =
              GetHealthBarColor(health.current_hp, health.regenerable_max_hp);
        }

        ESPRenderer::DrawLine( // current health
            ImVec2(Health1.x, Health1.y),
            ImVec2(currentHpPos.x, currentHpPos.y), healthColor, lineHeight);
        ESPRenderer::DrawLine( // regenerable black health
            ImVec2(currentHpPos.x, currentHpPos.y),
            ImVec2(currentMaxHpPos.x, currentMaxHpPos.y),
            ImVec4(0.039215f, 0.039215f, 0.039215f, 1.0f), lineHeight);
        ESPRenderer::DrawLine( // burning health
            ImVec2(currentMaxHpPos.x, currentMaxHpPos.y),
            ImVec2(potentialMaxHpPos.x, potentialMaxHpPos.y),
            ImVec4(0.784313f, 0.392156f, 0.039215f, 1.0f), lineHeight);
        ESPRenderer::DrawLine( // lost health
            ImVec2(potentialMaxHpPos.x, potentialMaxHpPos.y),
            ImVec2(Health2.x, Health2.y),
            ImVec4(0.784313f, 0.784313f, 0.784313f, 1.0f), lineHeight);
      }

      // ── Skeleton ESP ──────────────────────────────────────────────────────
      bool isFriend = (ent->GetType() == EntityType::FriendlyPlayer);
      bool drawBonesNow = (!isFriend && Configs.Player.DrawBones && !isDead)
                       || (isFriend  && Configs.Player.DrawBonesFriend);
      if (drawBonesNow)
      {
        static const int connections[][2] = {
          {0, 1}, {1, 2},           // head-neck-pelvis (spine)
          {1, 3}, {3, 4}, {4, 5},   // R arm
          {1, 9}, {9,10},{10,11},   // L arm
          {2, 6}, {6, 7}, {7, 8},   // R leg
          {2,12},{12,13},{13,14},   // L leg
        };
        auto boneColour = isFriend ? Configs.Player.FriendBonesColor : Configs.Player.BonesColor;
        for (auto& c : connections)
        {
          Vector3 a = ent->GetBonePosition(c[0]);
          Vector3 b = ent->GetBonePosition(c[1]);
          if (a.IsZero() || b.IsZero()) continue;
          Vector2 sa = CameraInstance->WorldToScreen(a, false);
          Vector2 sb = CameraInstance->WorldToScreen(b, false);
          if (sa.IsZero() || sb.IsZero()) continue;
          ESPRenderer::DrawLine(ImVec2(sa.x, sa.y), ImVec2(sb.x, sb.y), boneColour, 1.0f);
        }
      }

      if (!Configs.Player.Enable ||
          ent->GetType() == EntityType::FriendlyPlayer)
        continue;

      std::string wname = (Configs.Player.Name || isDead)
                              ? LOC("entity", ent->GetTypeAsString())
                              : "";
      std::string wdistance =
          Configs.Player.Distance
              ? std::vformat(LOC("menu", "esp.Meters"),
                             std::make_format_args(distance))
              : "";
      std::string whealth =
          Configs.Player.HP
              ? std::to_string(ent->GetHealth().current_hp) + "/" +
                    std::to_string(ent->GetHealth().current_max_hp) + "[" +
                    std::to_string(ent->GetHealth().regenerable_max_hp) + "]"
              : "";
      ESPRenderer::DrawText(ImVec2(feetPos.x, feetPos.y),
                            wname + wdistance + "\n" + whealth,
                            ent->GetType() == EntityType::FriendlyPlayer
                                ? Configs.Player.FriendColor
                                : Configs.Player.TextColor,
                            Configs.Player.FontSize, TopCenter);

    }
  }
}

bool IsValidHP(int hp) { return hp > 0 && hp <= 150; }