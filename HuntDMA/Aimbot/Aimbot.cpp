#include "Pch.h"
#include "Aimbot.h"
#include "PlayerEsp.h"
#include "Globals.h"
#include "CheatFunction.h"
#include "ConfigInstance.h"
#include "WorldEntity.h"
#include "ConfigUtilities.h"
#include "Makcu.h"
#include "InputManager.h"
#include "ESPRenderer.h"

static Vector2 GetCenterOfScreen()
{
	if (Configs.General.CrosshairLowerPosition)
		return Vector2(ESPRenderer::GetScreenWidth() * 0.5f, ESPRenderer::GetScreenHeight() * 0.6f);
	else
		return Vector2(ESPRenderer::GetScreenWidth() * 0.5f, ESPRenderer::GetScreenHeight() * 0.5f);
}

int ConditionalSwapPlayer(std::vector<std::shared_ptr<WorldEntity>>& entities, int low, int high)
{
	std::shared_ptr<WorldEntity> pivot = entities[high];
	int i = low - 1;
	Vector2 Centerofscreen = GetCenterOfScreen();
	for (int j = low; j < high; ++j)
	{
		if (Configs.Aimbot.Priority == 2)
		{
			if (Vector2::Distance(Centerofscreen, CameraInstance->WorldToScreen(entities[j]->GetPosition())) < Vector2::Distance(Centerofscreen, CameraInstance->WorldToScreen(pivot->GetPosition())))
			{
				++i;
				std::swap(entities[i], entities[j]);
				continue;
			}
			if (Vector3::Distance(CameraInstance->GetPosition(), entities[j]->GetPosition()) < Vector3::Distance(CameraInstance->GetPosition(), pivot->GetPosition()))
			{
				++i;
				std::swap(entities[i], entities[j]);
			}
		}
		if (Configs.Aimbot.Priority == 0)
		{
			if (Vector3::Distance(CameraInstance->GetPosition(), entities[j]->GetPosition()) < Vector3::Distance(CameraInstance->GetPosition(), pivot->GetPosition()))
			{
				++i;
				std::swap(entities[i], entities[j]);
			}
		}
		if (Configs.Aimbot.Priority == 1)
		{
			if (Vector2::Distance(Centerofscreen, CameraInstance->WorldToScreen(entities[j]->GetPosition())) < Vector2::Distance(Centerofscreen, CameraInstance->WorldToScreen(pivot->GetPosition())))
			{
				++i;
				std::swap(entities[i], entities[j]);
			}
		}
	}
	std::swap(entities[i + 1], entities[high]);
	return i + 1;
}

void QuickSortPlayers(std::vector<std::shared_ptr<WorldEntity>>& entities, int low, int high)
{
	if (low < high)
	{
		int pi = ConditionalSwapPlayer(entities, low, high);
		QuickSortPlayers(entities, low, pi - 1);
		QuickSortPlayers(entities, pi + 1, high);
	}
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
	if (CameraInstance->WorldToScreen(AimbotTarget->GetPosition()) == Vector2::Zero())
		return false;
	if (AimbotTarget->GetType() == EntityType::EnemyPlayer && !Configs.Aimbot.TargetPlayers)
		return false;
	if (Vector2::Distance(CameraInstance->WorldToScreen(AimbotTarget->GetPosition()), Centerofscreen) > Configs.Aimbot.FOV)
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
	Vector3 localpos = CameraInstance->GetPosition();
	templist = EnvironmentInstance->GetPlayerList();
	if (templist.size() == 0)
	{
		return;
	}

	QuickSortPlayers(templist, 0, templist.size() - 1);

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
		if (CameraInstance->WorldToScreen(player->GetPosition()) == Vector2::Zero())
			continue;
		if (Vector2::Distance(CameraInstance->WorldToScreen(player->GetPosition()), Centerofscreen) >Configs.Aimbot.FOV)
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
	
	// Check both keyboard and Makcu button
	bool keyboardHeld = Keyboard::IsKeyDown(Configs.Aimbot.AimKey);
	bool makcuHeld = Makcu::connected && Makcu::button_pressed(Configs.Aimbot.AimKey);
	
	AimKeyDown = (keyboardHeld || makcuHeld);
});

std::chrono::system_clock::time_point KmboxStart;

void Aimbot()
{  
	UpdateAimKey->Execute();
	if (!Makcu::connected || !AimKeyDown)
	{
		AimbotTarget = nullptr;
		return;
	}
	
	GetAimbotTarget();
	if (AimbotTarget == nullptr)
		return;
		
	Vector3 targetPos = AimbotTarget->GetPosition();
	if (targetPos == Vector3::Zero())
	{
		AimbotTarget = nullptr;
		return;
	}
	
	// Aim at head if enabled - using EXACT same calculation as head circle
	if (Configs.Aimbot.AimAtHead)
	{
		targetPos.z += 1.7f;  // Add head height offset (same as PlayerEsp.cpp line 114)
	}
	
	Vector2 screenpos = CameraInstance->WorldToScreen(targetPos);
	
	// Apply head circle X/Y offsets to match the drawn circle EXACTLY
	if (Configs.Aimbot.AimAtHead)
	{
		screenpos.x += Configs.Player.HeadCircleOffsetX;  // Use same offsets as head circle
		screenpos.y += Configs.Player.HeadCircleOffsetY;  // This ensures aimbot aims at circle center
	}
	
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
		
	if (KmboxStart + std::chrono::milliseconds(55) < std::chrono::system_clock::now())
	{
		Makcu::move((int)x, (int)y);
		KmboxStart = std::chrono::system_clock::now();
	}
}