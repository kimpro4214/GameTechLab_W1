#pragma once

#include <xinput.h>
#include <winerror.h>
#include <minwinbase.h>
#include <ImGui/imgui.h>

#pragma comment(lib, "Xinput.lib")

struct FController {
	XINPUT_STATE State;
	XINPUT_STATE PrevState;
	bool bIsConnected;
	float MoveValueLX;
	float MoveValueRX;
};

class GamepadManager
{
public:
	// 싱글톤이라 복사/대입 금지
	GamepadManager(const GamepadManager&) = delete;
	GamepadManager& operator=(const GamepadManager&) = delete;

	static GamepadManager& GetInstance();

	void	Initialize();
	void	Shutdown();
	bool	Update();
	bool	IsPushLeftStick();
	bool	IsPushRightStick();
	void	AddVibration();
	float	GetMoveValueLX();
	float	GetMoveValueRX();
	bool	IsButtonAPush();
	bool	IsButtonBPush();
	bool	IsDpadPushed();
	void	UpdateVibration();

private:
	GamepadManager();
	~GamepadManager();

	FController	FControllers[XUSER_MAX_COUNT];
	int			ActiveControllerIndex;
	void		ImGuiMapping();
	float		Intensity = 0.0f;
	bool		bIsVibrationActive = false;

	std::chrono::steady_clock::time_point EndTime;
	
	static constexpr float	MoveSensitivity = 0.01f;
	static constexpr int VibrationDurationMs = 150;         // 지속 시간 (밀리초)
	static constexpr float VibrationStepPerMerge = 0.25f;   // 합쳐질 때마다 증가하는 세기
	static constexpr float VibrationMaxIntensity = 1.0f;    // 최대 세기
};
