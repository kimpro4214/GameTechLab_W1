#pragma once

#include "Game/GameInput.h"

class GameSession;

class GameController
{
public:
	void HandleInput(GameSession& Session, const FGameInput& Input);
	void Reset();

private:
	bool bIsDraggingBall = false;
};
