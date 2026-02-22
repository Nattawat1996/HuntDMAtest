#include "Pch.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <utility>
#include "Aimbot.h"
#include "PlayerEsp.h"
#include "Globals.h"
#include "CheatFunction.h"
#include "ConfigInstance.h"
#include "WorldEntity.h"
#include "ConfigUtilities.h"
#include "Kmbox.h"
#include "InputManager.h"
#include "ESPRenderer.h"
#include "WeaponPresets.h"

// =============================================================================
//  Helpers
// =============================================================================

static Vector2 GetCenterOfScreen()
{
	if (Configs.General.CrosshairLowerPosition)
		return Vector2(ESPRenderer::GetScreenWidth() * 0.5f, ESPRenderer::GetScreenHeight() * 0.6f);
	else
		return Vector2(ESPRenderer::GetScreenWidth() * 0.5f, ESPRenderer::GetScreenHeight() * 0.5f);
}

// Lightweight head screen position — no prediction, safe for target selection & sticky check
static Vector2 GetHeadScreenPosition(std::shared_ptr<WorldEntity> entity)
{
	if (!entity || !CameraInstance)
		return Vector2::Zero();

	Vector3 headPos = entity->GetHeadPosition();  // uses bone head if available

	Vector2 screenPos = CameraInstance->WorldToScreen(headPos);
	return screenPos;
}

// =============================================================================
//  Cached ballistic parameters — resolved once when WeaponPreset changes,
//  not on every frame. weapons.find() is O(log N) string compare → too slow.
// =============================================================================

struct BallisticCache
{
	int     LastPreset  = -1;   // sentinel: force update on first use
	float   BulletSpeed = 500.0f;
	float   Drag        = 0.0f;
	float   DropMult    = 1.0f;
	bool    Valid       = false;
};
static BallisticCache gBallistic;

static void RefreshBallisticCache(int presetIndex)
{
	if (gBallistic.LastPreset == presetIndex) return;  // nothing changed

	gBallistic.LastPreset  = presetIndex;
	gBallistic.BulletSpeed = Configs.Aimbot.BulletSpeed;
	gBallistic.Drag        = 0.0f;
	gBallistic.DropMult    = 1.0f;
	gBallistic.Valid       = false;

	if (presetIndex > 0)
	{
		const char* wName = weaponNames[presetIndex - 1];
		auto it = weapons.find(wName);
		if (it != weapons.end())
		{
			const Weapon& w        = it->second;
			gBallistic.BulletSpeed = w.muzzleVelocity;
			gBallistic.Drag        = AmmoDrag[w.ammoType];

			float dropAdd  = DropAdd.count(w.ammoType) ? DropAdd[w.ammoType] : 0.0f;
			float actMult  = ActionMult[w.action];
			float barMult  = barrelMult[w.slot];
			float pre      = 1.0f + dropAdd + actMult + barMult;
			gBallistic.DropMult = std::max(pre, 0.25f);
			gBallistic.Valid    = true;
		}
	}
}


static Vector2 GetPredictedHeadScreenPosition(std::shared_ptr<WorldEntity> entity)
{
	if (!entity || !CameraInstance)
		return Vector2::Zero();

	Vector3 headPos = entity->GetHeadPosition();  // uses bone head if available

	if (Configs.Aimbot.Prediction)
	{
		// NOTE: UpdateVelocity() called in UpdatePlayers->Execute() — not here.

		float distance = Vector3::Distance(CameraInstance->GetPosition(), headPos);

		// ── Resolve ballistic parameters (cached — only recalculated when preset changes) ──
		RefreshBallisticCache(Configs.Aimbot.WeaponPreset);
		// Also update cache if manual BulletSpeed changed while preset==0
		if (Configs.Aimbot.WeaponPreset == 0)
			gBallistic.BulletSpeed = Configs.Aimbot.BulletSpeed;

		float bulletSpeed = gBallistic.BulletSpeed;
		float drag        = gBallistic.Drag;
		float dropMult    = gBallistic.DropMult;

		if (bulletSpeed > 0.0f && distance > 0.0f)
		{
			float travelTime = distance / bulletSpeed;

			// ── Movement lead ──────────────────────────────────────────────
			float dragFactor = std::max(1.0f - drag * distance, 0.1f);
			Vector3 offset   = entity->Velocity * (travelTime * Configs.Aimbot.PredictionScale * dragFactor);

			// ── Bullet drop  s = 0.5 * g * t^2 * dropMult ────────────────
			float gravity = 9.81f * Configs.Aimbot.GravityScale;
			float drop    = 0.5f * gravity * travelTime * travelTime * dropMult;
			offset.z     += drop;

			headPos = headPos + offset;
		}
	}


	Vector2 screenPos = CameraInstance->WorldToScreen(headPos);
	return screenPos;
}

// =============================================================================
//  Target Selection
//  O(N) WorldToScreen calls (cached before sort) — comparator is O(1).
// =============================================================================

// Per-candidate scoring cache — filled once before std::sort
struct ScoredEntity
{
	std::shared_ptr<WorldEntity> Entity;
	float Score;
};

static void SortPlayers(std::vector<std::shared_ptr<WorldEntity>>& entities)
{
	if (entities.empty() || !CameraInstance) return;

	Vector2 center = GetCenterOfScreen();
	Vector3 camPos = CameraInstance->GetPosition();

	// Pre-score every entity O(N) — WorldToScreen called once per entity max
	std::vector<ScoredEntity> scored;
	scored.reserve(entities.size());

	for (auto& e : entities)
	{
		if (!e) continue;

		float score = 0.0f;
		if (Configs.Aimbot.Priority == 0) // Distance
		{
			score = Vector3::Distance(camPos, e->GetPosition());
		}
		else // Crosshair (1) or Both (2) — need screen pos
		{
			Vector2 scrPos  = CameraInstance->WorldToScreen(e->GetPosition()); // called ONCE
			float   scrDist = Vector2::Distance(center, scrPos);

			if (Configs.Aimbot.Priority == 1)
				score = scrDist;
			else // Both
				score = Vector3::Distance(camPos, e->GetPosition()) + scrDist;
		}
		scored.push_back({ e, score });
	}

	// O(1) comparator — no WorldToScreen inside
	std::sort(scored.begin(), scored.end(),
		[](const ScoredEntity& a, const ScoredEntity& b) { return a.Score < b.Score; });

	for (size_t i = 0; i < entities.size(); ++i)
		entities[i] = scored[i].Entity;
}

// =============================================================================
//  Sticky target & target acquisition
// =============================================================================

std::shared_ptr<WorldEntity> AimbotTarget;

bool StickTarget()
{
	Vector2 center = GetCenterOfScreen();
	if (!CameraInstance || !EnvironmentInstance || !AimbotTarget) return false;
	if (!AimbotTarget->GetValid()) return false;
	if (Vector3::Distance(CameraInstance->GetPosition(), AimbotTarget->GetPosition()) > Configs.Aimbot.MaxDistance)
		return false;
	if (AimbotTarget->GetType() == EntityType::EnemyPlayer && !Configs.Aimbot.TargetPlayers)
		return false;

	Vector2 headPos = GetHeadScreenPosition(AimbotTarget);
	if (headPos == Vector2::Zero()) return false;
	// Wider FOV for sticky (1.5×) prevents losing target at FOV edge
	if (Vector2::Distance(headPos, center) > Configs.Aimbot.FOV * 1.5f) return false;
	return true;
}

// Rate-limited target selection (called via CheatFunction, not every frame)
static std::shared_ptr<CheatFunction> UpdateTarget = std::make_shared<CheatFunction>(50, [] {
	if (!CameraInstance || !EnvironmentInstance) return;
	if (!Configs.Aimbot.Enable) return;
	if (StickTarget()) return;   // keep existing target — skip expensive re-sort

	Vector2 center = GetCenterOfScreen();

	auto templist = EnvironmentInstance->GetPlayerList();
	if (templist.empty()) { AimbotTarget = nullptr; return; }

	SortPlayers(templist);

	for (auto& player : templist)
	{
		if (!player) continue;
		if (!Configs.Aimbot.TargetPlayers) continue;
		if (!player->GetValid()) continue;
		if (player->GetType() == EntityType::FriendlyPlayer) continue;
		if (player->GetType() == EntityType::LocalPlayer)    continue;
		if (Vector3::Distance(CameraInstance->GetPosition(), player->GetPosition()) > Configs.Aimbot.MaxDistance)
			continue;

		Vector2 headPos = GetHeadScreenPosition(player);
		if (headPos == Vector2::Zero()) continue;
		if (Vector2::Distance(headPos, center) > Configs.Aimbot.FOV) continue;

		AimbotTarget = player;
		return;
	}
	AimbotTarget = nullptr;
});

// =============================================================================
//  Aim key state
// =============================================================================

bool AimKeyDown = false;

std::shared_ptr<CheatFunction> UpdateAimKey = std::make_shared<CheatFunction>(50, [] {
	if (!EnvironmentInstance) return;
	if (EnvironmentInstance->GetObjectCount() < 10) return;
	AimKeyDown = Keyboard::IsKeyDown(Configs.Aimbot.Aimkey) || kmbox::IsDown(Configs.Aimbot.Aimkey);
});

// =============================================================================
//  Axis-Unlock Anti-Detection
//  Perfect horizontal/vertical movements are inhuman — inject perpendicular noise.
// =============================================================================

static void ApplyAxisUnlock(float& x, float& y, float distToTarget)
{
	if (!Configs.Aimbot.AxisLockEnable) return;  // feature toggle

	// Don't inject noise when locked on — it causes scope wobble
	const float LOCK_PX = 20.0f;
	if (distToTarget < LOCK_PX) return;

	float absX    = std::abs(x);
	float absY    = std::abs(y);
	float maxAxis = std::max(absX, absY);
	if (maxAxis <= 1.0f) return;  // insignificant movement — skip

	float axisRatio = std::min(absX, absY) / maxAxis;
	if (axisRatio >= Configs.Aimbot.AxisLockThreshold) return;  // not axis-locked

	// Probabilistic injection
	static std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
	if (chanceDist(rng) >= Configs.Aimbot.AxisNoiseChance) return;

	std::uniform_real_distribution<float> noiseDist(Configs.Aimbot.AxisNoiseMin, Configs.Aimbot.AxisNoiseMax);
	float perpNoise = noiseDist(rng);
	float sign      = (chanceDist(rng) > 0.5f) ? 1.0f : -1.0f;

	if (absX < absY)
		x += sign * perpNoise;  // mostly vertical   → add horizontal noise
	else
		y += sign * perpNoise;  // mostly horizontal → add vertical noise
}

// =============================================================================
//  Main Aimbot loop
// =============================================================================

std::chrono::system_clock::time_point KmboxStart;

void Aimbot()
{
	UpdateAimKey->Execute();
	if (!kmbox::connected || !AimKeyDown)
	{
		AimbotTarget = nullptr;
		return;
	}

	// Rate-limited target selection (50 ms)
	UpdateTarget->Execute();

	if (!AimbotTarget) return;
	if (AimbotTarget->GetPosition() == Vector3::Zero())
	{
		AimbotTarget = nullptr;
		return;
	}

	// Get ballistic-predicted screen position (1 WorldToScreen call total)
	Vector2 screenpos    = GetPredictedHeadScreenPosition(AimbotTarget);
	Vector2 Centerscreen = GetCenterOfScreen();

	if (screenpos == Vector2::Zero()) { AimbotTarget = nullptr; return; }
	if (Vector2::Distance(screenpos, Centerscreen) > Configs.Aimbot.FOV) return;

	float x = screenpos.x - Centerscreen.x;
	float y = screenpos.y - Centerscreen.y;

	// ── Smoothing (S-curve zones) ─────────────────────────────────────────
	float distToTarget  = std::sqrt(x * x + y * y);
	float smoothing     = std::max(Configs.Aimbot.Smoothing, 1.0f);
	const float LOCK_PX = 20.0f;

	// In DMA aimbots, 1.0f smoothing causes severe oscillation (shaking) due to memory read latency.
	// We dynamically reduce smoothing in the lock zone but enforce a minimum limit to prevent overcorrecting.
	float minSafeSmoothing = 1.5f;

	if (distToTarget < LOCK_PX)
		smoothing = std::max(smoothing * 0.5f, minSafeSmoothing);
	else if (distToTarget < LOCK_PX * 2.0f)
	{
		float t = (distToTarget - LOCK_PX) / LOCK_PX;
		float targetSmooth = std::max(smoothing * 0.5f, minSafeSmoothing);
		smoothing = targetSmooth + (smoothing - targetSmooth) * t;
	}
	if (smoothing < minSafeSmoothing) smoothing = minSafeSmoothing;

	x /= smoothing;
	y /= smoothing;

	// ── Axis-Unlock Anti-Detection (skips when locked to prevent scope wobble) ──
	ApplyAxisUnlock(x, y, distToTarget);

	// ── Mouse output ──────────────────────────────────────────────────────────
	// Note: DMA updates at ~60Hz (16.6ms). Sending sub-16ms updates causes double-correction overshoot.
	// We cap the update rate to at least 16ms to synchronize with memory updates.
	const int lockUpdateRateMs    = std::max(16, Configs.Aimbot.UpdateRate / 2);
	const int normalUpdateRateMs  = std::max(16, Configs.Aimbot.UpdateRate);
	const int updateRateMs        = (distToTarget < LOCK_PX) ? lockUpdateRateMs : normalUpdateRateMs;

	if (KmboxStart + std::chrono::milliseconds(updateRateMs) < std::chrono::system_clock::now())
	{
		static float residualX = 0.0f;
		static float residualY = 0.0f;

		float targetX = x + residualX;
		float targetY = y + residualY;

		// In lock zone: almost no deadzone so every sub-pixel is corrected
		float baseThreshold = 0.5f + (Configs.Aimbot.Stability * 0.5f);
		if (distToTarget < LOCK_PX) baseThreshold = 0.05f;

		if (std::abs(targetX) < baseThreshold && std::abs(targetY) < baseThreshold)
		{
			residualX = targetX * 0.9f;
			residualY = targetY * 0.9f;
		}
		else
		{
			int moveX = (int)targetX;
			int moveY = (int)targetY;
			residualX = targetX - moveX;
			residualY = targetY - moveY;

			if (moveX != 0 || moveY != 0)
				kmbox::move(moveX, moveY);
		}

		KmboxStart = std::chrono::system_clock::now();
	}
}