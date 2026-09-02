#pragma once

#include <xinput.h>
#include <winerror.h>
#include <minwinbase.h>

#pragma comment(lib, "Xinput.lib")

struct FController {
	XINPUT_STATE State;
	bool bIsConnected;

	float MoveValueLX;
	float MoveValueRX;
};

class GamepadInputManager
{
public:
	// 싱글톤이라 복사/대입 금지
	GamepadInputManager(const GamepadInputManager&) = delete;
	GamepadInputManager& operator=(const GamepadInputManager&) = delete;

	static GamepadInputManager& GetInstance();

	void	Initialize();
	void	Shutdown();
	bool	Update();
	bool	IsPushLeftStick();
	bool	IsPushRightStick();
	void	SetVibration(bool status);
	float	GetMoveValueLX();
	float	GetMoveValueRX();
	bool	IsButtonAPush();
	bool	IsButtonBPush();

private:
	GamepadInputManager();
	~GamepadInputManager();

	FController				FControllers[XUSER_MAX_COUNT];
	int						ActiveControllerIndex;
	static constexpr float	MoveSensitivity = 0.01f;
};
