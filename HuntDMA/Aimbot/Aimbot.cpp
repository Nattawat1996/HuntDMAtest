#include "Pch.h"
#include <cmath>
#include <algorithm>
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

static Vector2 GetCenterOfScreen()
{
	if (Configs.General.CrosshairLowerPosition)
		return Vector2(ESPRenderer::GetScreenWidth() * 0.5f, ESPRenderer::GetScreenHeight() * 0.6f);
	else
		return Vector2(ESPRenderer::GetScreenWidth() * 0.5f, ESPRenderer::GetScreenHeight() * 0.5f);
}

// Helper function to get the predicted head screen position
static Vector2 GetPredictedHeadScreenPosition(std::shared_ptr<WorldEntity> entity)
{
	if (!entity || !CameraInstance)
		return Vector2::Zero();
	
	// Calculate head position (base position + 1.7m height)
	Vector3 headPos = entity->GetPosition();
	headPos.z += 1.7f;
	
	// Apply prediction if enabled
	if (Configs.Aimbot.Prediction)
	{
		// Update velocity tracking
		entity->UpdateVelocity();
		
		float distance = Vector3::Distance(CameraInstance->GetPosition(), headPos);
		
		// Look up weapon data from the weapons map
		float bulletSpeed = Configs.Aimbot.BulletSpeed;
		float drag = 0.0f;
		float ammo = 0.0f;
		int droprange = (int)Configs.Aimbot.DropRange;
		float action = 0.0f;
		float slot = 0.0f;
		bool weaponFound = false;
		std::string weaponUsed = "Custom";
		
		if (Configs.Aimbot.WeaponPreset > 0) {
			std::string inputWeapon = weaponNames[Configs.Aimbot.WeaponPreset - 1];
			auto it = weapons.find(inputWeapon);
			if (it != weapons.end()) {
				const Weapon& weapon = it->second;
				bulletSpeed = weapon.muzzleVelocity;
				drag = AmmoDrag[weapon.ammoType];
				ammo = DropAdd.count(weapon.ammoType) ? DropAdd[weapon.ammoType] : 0.0f;
				droprange = weapon.dropRange;
				action = ActionMult[weapon.action];
				slot = barrelMult[weapon.slot];
				weaponFound = true;
				weaponUsed = inputWeapon;
			} else {
				weaponUsed = inputWeapon + " (NOT FOUND!)";
			}
		}
		
		if (bulletSpeed > 0.0f && distance > 0.0f)
		{
			// Composite drop multiplier from ammo, action, and slot
			float predropmult = 1.0f + ammo + action + slot;
			float dropmult = std::max(predropmult, 0.25f);
			
			float travelTime = distance / bulletSpeed;
			
			// Predict position with drag dampening on velocity
			Vector3 predicted = entity->Velocity * (travelTime * Configs.Aimbot.PredictionScale * (1.0f - drag));
			
			// Bullet drop: 0.5 * gravity * t^2 * dropmult (gravity = 9.81 * GravityScale)
			float gravity = 9.81f * Configs.Aimbot.GravityScale;
			float drop = (0.5f * gravity * travelTime * travelTime) * dropmult;
			predicted.z += drop;
			
			// Debug logging (throttled to once per second)
			static auto lastLogTime = std::chrono::system_clock::now();
			auto now = std::chrono::system_clock::now();
			if (now - lastLogTime > std::chrono::seconds(1)) {
				LOG_INFO("[Aimbot] Weapon: %s | Preset: %d | Found: %s",
					weaponUsed.c_str(), Configs.Aimbot.WeaponPreset, weaponFound ? "YES" : "NO");
				LOG_INFO("[Aimbot] Speed=%.1f | Drag=%.2f | DropAdd=%.2f | Action=%.2f | Slot=%.2f | DropRange=%d",
					bulletSpeed, drag, ammo, action, slot, droprange);
				LOG_INFO("[Aimbot] Dist=%.1fm | TravelT=%.3fs | DropMult=%.2f | Gravity=%.2f | Drop=%.3fm",
					distance, travelTime, dropmult, gravity, drop);
				lastLogTime = now;
			}
			
			headPos = headPos + predicted;
		}
	}
	
	// Convert to screen space
	Vector2 screenPos = CameraInstance->WorldToScreen(headPos);
	
	// Apply screen space offsets to match the drawn head circle center
	screenPos.x += Configs.Player.HeadCircleOffsetX;
	screenPos.y += Configs.Player.HeadCircleOffsetY;
	
	return screenPos;
}

// Lightweight head screen position (no prediction/velocity — safe for target selection)
static Vector2 GetHeadScreenPosition(std::shared_ptr<WorldEntity> entity)
{
	if (!entity || !CameraInstance)
		return Vector2::Zero();
	
	Vector3 headPos = entity->GetPosition();
	headPos.z += 1.7f;
	
	Vector2 screenPos = CameraInstance->WorldToScreen(headPos);
	screenPos.x += Configs.Player.HeadCircleOffsetX;
	screenPos.y += Configs.Player.HeadCircleOffsetY;
	
	return screenPos;
}

// Optimized sort: uses std::sort with cached positions (avoids redundant WorldToScreen calls)
static void SortPlayers(std::vector<std::shared_ptr<WorldEntity>>& entities)
{
	if (entities.empty() || !CameraInstance) return;

	Vector2 center = GetCenterOfScreen();
	Vector3 camPos = CameraInstance->GetPosition();

	std::sort(entities.begin(), entities.end(),
		[&center, &camPos](const std::shared_ptr<WorldEntity>& a, const std::shared_ptr<WorldEntity>& b)
		{
			if (Configs.Aimbot.Priority == 0) // Distance
			{
				return Vector3::Distance(camPos, a->GetPosition()) < Vector3::Distance(camPos, b->GetPosition());
			}
			else if (Configs.Aimbot.Priority == 1) // Crosshair
			{
				return Vector2::Distance(center, CameraInstance->WorldToScreen(a->GetPosition()))
					 < Vector2::Distance(center, CameraInstance->WorldToScreen(b->GetPosition()));
			}
			else // Both
			{
				float distA = Vector3::Distance(camPos, a->GetPosition());
				float distB = Vector3::Distance(camPos, b->GetPosition());
				float scrA = Vector2::Distance(center, CameraInstance->WorldToScreen(a->GetPosition()));
				float scrB = Vector2::Distance(center, CameraInstance->WorldToScreen(b->GetPosition()));
				return (distA + scrA) < (distB + scrB);
			}
		}
	);
}

std::shared_ptr<WorldEntity> AimbotTarget;

bool StickTarget()
{
	Vector2 Centerofscreen = GetCenterOfScreen();
	if (CameraInstance == nullptr)
		return false;
	if (EnvironmentInstance == nullptr)
		return false;
	if (AimbotTarget== nullptr)
		return false;	
	if (!AimbotTarget->GetValid())
		return false;
	if (Vector3::Distance(CameraInstance->GetPosition(), AimbotTarget->GetPosition()) > Configs.Aimbot.MaxDistance)
		return false;
	if (AimbotTarget->GetType() == EntityType::EnemyPlayer && !Configs.Aimbot.TargetPlayers)
		return false;
	// Lightweight FOV check (no prediction/velocity — that's only in the main Aimbot() loop)
	Vector2 headScreenPos = GetHeadScreenPosition(AimbotTarget);
	if (headScreenPos == Vector2::Zero())
		return false;
	// Use wider FOV for sticky target (1.5x) to prevent losing target during movement
	if (Vector2::Distance(headScreenPos, Centerofscreen) > Configs.Aimbot.FOV * 1.5f)
		return false;
	return true;
}

void GetAimbotTarget()
{
	if (CameraInstance == nullptr)
		return;
	if (EnvironmentInstance == nullptr)
		return;
	if (!Configs.Aimbot.Enable)
		return;
	if(StickTarget())
		return;
	Vector2 Centerofscreen = GetCenterOfScreen();

	std::vector<std::shared_ptr<WorldEntity>> templist;
	templist = EnvironmentInstance->GetPlayerList();
	if (templist.empty())
		return;

	SortPlayers(templist);

	for (std::shared_ptr<WorldEntity> player : templist)
	{
		if(player == nullptr)
			continue;
		if (!Configs.Aimbot.TargetPlayers)
			continue;
		if (!player->GetValid())
			continue;
		if (player->GetType() == EntityType::FriendlyPlayer)
			continue;
		if (player->GetType() == EntityType::LocalPlayer)
			continue;
		if (Vector3::Distance(CameraInstance->GetPosition(), player->GetPosition()) > Configs.Aimbot.MaxDistance)
			continue;
		// Check if adjusted head position is within FOV
		Vector2 headScreenPos = GetHeadScreenPosition(player);
		if (headScreenPos == Vector2::Zero())
			continue;
		if (Vector2::Distance(headScreenPos, Centerofscreen) > Configs.Aimbot.FOV)
			continue;
		AimbotTarget = player;
		return;
	}
	AimbotTarget = nullptr;
}

bool AimKeyDown = false;

std::shared_ptr<CheatFunction> UpdateAimKey = std::make_shared<CheatFunction>(50, [] {
	if (EnvironmentInstance == nullptr)
		return;
	if (EnvironmentInstance->GetObjectCount() < 10)
		return;
	if (Keyboard::IsKeyDown(Configs.Aimbot.Aimkey) || kmbox::IsDown(Configs.Aimbot.Aimkey))
	{
		AimKeyDown = true;
	}
	else
	{
		AimKeyDown = false;
	}
});

std::chrono::system_clock::time_point KmboxStart;

void Aimbot()
{  
	UpdateAimKey->Execute();
	if (!kmbox::connected || !AimKeyDown)
	{
		AimbotTarget = nullptr;
		return;
	}
	
	GetAimbotTarget();
	if (AimbotTarget == nullptr)
		return;
	
	if (AimbotTarget->GetPosition() == Vector3::Zero())
	{
		AimbotTarget = nullptr;
		return;
	}

	// Get predicted screen position (includes velocity tracking, latency comp, bullet travel, drop)
	Vector2 screenpos = GetPredictedHeadScreenPosition(AimbotTarget);

	Vector2 Centerofscreen = GetCenterOfScreen();
	if (Vector2::Distance(screenpos, Centerofscreen) > Configs.Aimbot.FOV)
		return;

	if (screenpos == Vector2::Zero())
	{
		AimbotTarget = nullptr;
		return;
	}
	
	float x = screenpos.x - Centerofscreen.x;
	float y = screenpos.y - Centerofscreen.y;
	
	// --- 5. Target Lock & Smoothing Logic ---
	float distanceToTarget = std::sqrt(x * x + y * y);
	float smoothing = Configs.Aimbot.Smoothing;

	// LOCK MODE: If we are very close to the target, disable smoothing to stick perfectly
	const float LOCK_THRESHOLD = 20.0f; // Pixels
	
	if (distanceToTarget < LOCK_THRESHOLD)
	{
		// Perfect Lock: Zero smoothing
		smoothing = 1.0f; 
	}
	else if (distanceToTarget < LOCK_THRESHOLD * 2.0f) 
	{
		// Transition Zone: Linear blend from 1.0 to ConfigSmoothing
		// Dist 20 -> 1.0
		// Dist 40 -> ConfigSmoothing
		float t = (distanceToTarget - LOCK_THRESHOLD) / LOCK_THRESHOLD;
		smoothing = 1.0f + (smoothing - 1.0f) * t;
	}

	// Additional Safety: Ensure we never divide by < 1
	if (smoothing < 1.0f) smoothing = 1.0f;
	
	x = x / smoothing;
	y = y / smoothing;
	

	if (KmboxStart + std::chrono::milliseconds(Configs.Aimbot.UpdateRate) < std::chrono::system_clock::now()) 
	{
		// Sub-Pixel Logic
		static float residualX = 0.0f;
		static float residualY = 0.0f;

		float targetX = x + residualX;
		float targetY = y + residualY;

		// Reduced deadzone in Lock Mode for responsiveness
		float baseThreshold = 0.5f + (Configs.Aimbot.Stability * 0.5f);
		if (distanceToTarget < LOCK_THRESHOLD) baseThreshold *= 0.1f; // Almost no deadzone when locked

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
			{
				kmbox::move(moveX, moveY);
			}
		}
		
		KmboxStart = std::chrono::system_clock::now();
	}
}