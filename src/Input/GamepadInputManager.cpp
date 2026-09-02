#include "pch.h"
#include "Input/GamepadInputManager.h"

GamepadManager::GamepadManager() : ActiveControllerIndex(-1)
{
    for (int i = 0; i < XUSER_MAX_COUNT; i++)
    {
        FControllers[i].bIsConnected = false;
        FControllers[i].State = {};
        FControllers[i].PrevState = {};
    }

}

GamepadManager::~GamepadManager()
{
    Shutdown();
}


GamepadManager& GamepadManager::GetInstance()
{
    static GamepadManager Instance;
    return Instance;
}

void GamepadManager::Initialize()
{
    Update();
}

void GamepadManager::Shutdown()
{
    if (ActiveControllerIndex != -1)
    {
        XINPUT_VIBRATION VibrationData = {};
        XInputSetState(ActiveControllerIndex, &VibrationData);
    }

    Intensity = 0.0f;
    bIsVibrationActive = false;
}


bool GamepadManager::IsPushLeftStick()
{
    if (ActiveControllerIndex == -1)
    {
        return (false);
    }
    XINPUT_STATE state = FControllers[ActiveControllerIndex].State;

    float LX = state.Gamepad.sThumbLX;

    float normalizedLX = LX;

    //check if the controller is outside a dead zone
    if (LX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
    {
        LX -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
    }
    else if (LX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
    {
        LX += XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
    }
    else //if the controller is in the deadzone zero out
    {
        return (false);
    }
    normalizedLX = LX / (32767.0f - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    FControllers[ActiveControllerIndex].MoveValueLX =
        normalizedLX * MoveSensitivity;

    return (true);
}

bool GamepadManager::IsPushRightStick()
{
    if (ActiveControllerIndex == -1)
    {
        return (false);
    }
    XINPUT_STATE state = FControllers[ActiveControllerIndex].State;

    float RX = state.Gamepad.sThumbRX;

    float normalizedRX = RX;

    //check if the controller is outside a dead zone
    if (RX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
    {
        RX -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
    }
    else if (RX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
    {
        RX += XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
    }
    else //if the controller is in the deadzone zero out
    {
        return (false);
    }
    normalizedRX = RX / (32767.0f - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    FControllers[ActiveControllerIndex].MoveValueRX =
        normalizedRX * MoveSensitivity;

    return (true);
}

bool GamepadManager::IsButtonAPush()
{
    if (ActiveControllerIndex == -1)
    {
        return (false);
    }


    const WORD Current = FControllers[ActiveControllerIndex].State.Gamepad.wButtons;
    const WORD Previous = FControllers[ActiveControllerIndex].PrevState.Gamepad.wButtons;

    const bool bIsDownNow = (Current & XINPUT_GAMEPAD_A) != 0;
    const bool bWasDownBefore = (Previous & XINPUT_GAMEPAD_A) != 0;

    return (bIsDownNow && !bWasDownBefore);   // 방금 눌린 순간만 true
}

bool GamepadManager::IsButtonBPush()
{
    if (ActiveControllerIndex == -1)
    {
        return (false);
    }

    const WORD Current = FControllers[ActiveControllerIndex].State.Gamepad.wButtons;
    const WORD Previous = FControllers[ActiveControllerIndex].PrevState.Gamepad.wButtons;

    const bool bIsDownNow = (Current & XINPUT_GAMEPAD_B) != 0;
    const bool bWasDownBefore = (Previous & XINPUT_GAMEPAD_B) != 0;

    return (bIsDownNow && !bWasDownBefore);
}

void GamepadManager::UpdateVibration()
{
    if (ActiveControllerIndex == -1 || !bIsVibrationActive)
    {
        return;
    }

    const auto Now = std::chrono::steady_clock::now();

    if (Now >= EndTime)
    {
        Intensity = 0.0f;
        bIsVibrationActive = false;

        XINPUT_VIBRATION VibrationData = {};
        XInputSetState(ActiveControllerIndex, &VibrationData);
        return;
    }

    XINPUT_VIBRATION VibrationData = {};
    VibrationData.wLeftMotorSpeed =
        static_cast<WORD>(Intensity * 65535);
    VibrationData.wRightMotorSpeed =
        static_cast<WORD>(Intensity * 65535 * 0.5f);
    XInputSetState(ActiveControllerIndex, &VibrationData);
}

void GamepadManager::AddVibration()
{
    if (ActiveControllerIndex == -1)
    {
        return;
    }

    Intensity = std::min(Intensity + VibrationStepPerMerge, VibrationMaxIntensity);

    EndTime = std::chrono::steady_clock::now() +
        std::chrono::milliseconds(VibrationDurationMs);

    bIsVibrationActive = true;
}


void GamepadManager::ImGuiMapping()
{
    ImGuiIO& io = ImGui::GetIO();
    
    XINPUT_STATE& state = FControllers[ActiveControllerIndex].State;

    auto MAP_BUTTON = [&](ImGuiKey key, int button)
        {
            io.AddKeyEvent(key, (state.Gamepad.wButtons & button) != 0);
        };
    
    // 4방향 D-Pad만 매핑
    MAP_BUTTON(ImGuiKey_GamepadDpadUp, XINPUT_GAMEPAD_DPAD_UP);
    MAP_BUTTON(ImGuiKey_GamepadDpadDown, XINPUT_GAMEPAD_DPAD_DOWN);
    MAP_BUTTON(ImGuiKey_GamepadDpadLeft, XINPUT_GAMEPAD_DPAD_LEFT);
    MAP_BUTTON(ImGuiKey_GamepadDpadRight, XINPUT_GAMEPAD_DPAD_RIGHT);

    // 선택/확인용 버튼도 최소한 하나는 있어야 UI 조작이 완성됨
    MAP_BUTTON(ImGuiKey_GamepadFaceDown, XINPUT_GAMEPAD_A);   // 선택(클릭 대응)
    MAP_BUTTON(ImGuiKey_GamepadFaceRight, XINPUT_GAMEPAD_B);  // 취소/뒤로가기
    
}

bool GamepadManager::Update()
{
    DWORD dwResult;
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
    {
        FControllers[i].PrevState = FControllers[i].State;

        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));

        dwResult = XInputGetState(i, &state);

        if (dwResult == ERROR_SUCCESS)
        {
            FControllers[i].State = state;
            FControllers[i].bIsConnected = true;
            ActiveControllerIndex = i;
            ImGuiMapping();
            UpdateVibration();
            return (true);
        }
        else
        {
            FControllers[i].State = {};
            FControllers[i].bIsConnected = false;
        }
    }
    ActiveControllerIndex = -1;
    return (false);
}

bool GamepadManager::IsDpadPushed()
{
    if (ActiveControllerIndex == -1) return false;

    const WORD Current = FControllers[ActiveControllerIndex].State.Gamepad.wButtons;
    const WORD Previous = FControllers[ActiveControllerIndex].PrevState.Gamepad.wButtons;

    const WORD DpadMask = XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN |
        XINPUT_GAMEPAD_DPAD_LEFT | XINPUT_GAMEPAD_DPAD_RIGHT;

    const bool bIsDownNow = (Current & DpadMask) != 0;
    const bool bWasDownBefore = (Previous & DpadMask) != 0;

    return bIsDownNow && !bWasDownBefore;
}

float GamepadManager::GetMoveValueLX()
{
    return (FControllers[ActiveControllerIndex].MoveValueLX);
}

float GamepadManager::GetMoveValueRX()
{
    return (FControllers[ActiveControllerIndex].MoveValueRX);
}

