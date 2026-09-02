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

	GamepadInputManager& RefGamepadInputManager = GamepadInputManager::GetInstance();
	if (GamepadInputManager::GetInstance().Update())
	{
		if (RefGamepadInputManager.IsPushLeftStick())
		{
			Session.GamepadMoveCurrentBall(RefGamepadInputManager.GetMoveValueLX());
		}
		if (RefGamepadInputManager.IsPushRightStick())
		{
			Session.GamepadMoveCurrentBall(RefGamepadInputManager.GetMoveValueRX());
		}
	}

	if (GamepadInputManager::GetInstance().IsButtonAPush())
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

	if (GamepadInputManager::GetInstance().IsButtonBPush() || 
		Input.bCanUseSceneMouse && Input.bIsRightMouseReleased)
	{
		Session.SwapCurrentBall();
	}
}

void GameController::Reset()
{
	bIsDraggingBall = false;
}
