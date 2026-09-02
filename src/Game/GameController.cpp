#include "pch.h"
#include "Game/GameController.h"

#include "Game/GameSession.h"

#include "Input/GamepadInputManager.h"

void GameController::HandleInput(GameSession& Session, const FGameInput& Input)
{
	if (Session.IsMainMenu() || Session.IsGameOver())
	{
		return;
	}

	GamepadManager& RefGamepadInputManager = GamepadManager::GetInstance();
	bool bIsNavActive = ImGui::GetIO().NavActive;

	// UI에 포커스가 있을 때 B를 누르면 게임으로 복귀
	if (bIsNavActive && RefGamepadInputManager.IsButtonBPush())
	{
		ImGui::SetWindowFocus(nullptr);
		return;
	}

	// 게임 화면에 있을 때 D-Pad → UI 창으로 진입
	if (!bIsNavActive && RefGamepadInputManager.IsDpadPushed())
	{
		ImGui::SetWindowFocus("UI");
		return;
	}

	if (!bIsNavActive && RefGamepadInputManager.IsPushLeftStick())
	{
		Session.GamepadMoveCurrentBall(RefGamepadInputManager.GetMoveValueLX());
	}
	if (!bIsNavActive && RefGamepadInputManager.IsPushRightStick())
	{
		Session.GamepadMoveCurrentBall(RefGamepadInputManager.GetMoveValueRX());
	}

	if (!bIsNavActive && GamepadManager::GetInstance().IsButtonAPush())
	{
		Session.DropCurrentBall();
	}
	else
	{
		const bool bCanMoveDraggedBall =
			Input.bIsLeftMouseDown && (Input.bCanUseSceneMouse || bIsDraggingBall);
		if (bCanMoveDraggedBall)
		{
			Session.MoveCurrentBall(Input.MouseWorldX);
			bIsDraggingBall = true;
		}

		if (bIsDraggingBall && Input.bIsLeftMouseReleased && Session.DropCurrentBall())
		{
			bIsDraggingBall = false;
		}
	}

	if (!bIsNavActive && GamepadManager::GetInstance().IsButtonBPush() ||
		Input.bCanUseSceneMouse && Input.bIsRightMouseReleased)
	{
		Session.SwapCurrentBall();
	}
}

void GameController::Reset()
{
	bIsDraggingBall = false;
	ImGui::SetWindowFocus(nullptr);
}
