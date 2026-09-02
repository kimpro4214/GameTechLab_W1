#include "pch.h"
#include "Input/GamepadInputManager.h"

GamepadInputManager::GamepadInputManager() : ActiveControllerIndex(-1)
{
    for (int i = 0; i < XUSER_MAX_COUNT; i++)
    {
        FControllers[i].bIsConnected = false;
        FControllers[i].State = {};
    }

}

GamepadInputManager::~GamepadInputManager()
{

}


GamepadInputManager& GamepadInputManager::GetInstance()
{
    static GamepadInputManager Instance;
    return Instance;
}

void GamepadInputManager::Initialize()
{
    Update();
}

void GamepadInputManager::Shutdown()
{

}


bool GamepadInputManager::IsPushLeftStick()
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

bool GamepadInputManager::IsPushRightStick()
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

void GamepadInputManager::SetVibration(bool status)
{
    if (ActiveControllerIndex == -1)
    {
        return;
    }

    XINPUT_VIBRATION vibration = {};
    if (status == true)
    {
        ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
        vibration.wLeftMotorSpeed = 32000; // use any value between 0-65535 here
        vibration.wRightMotorSpeed = 16000; // use any value between 0-65535 here
    }
        XInputSetState(ActiveControllerIndex, &vibration);
}

bool GamepadInputManager::Update()
{
    // 몇초마다 검사하게 수정하면 좋음

    DWORD dwResult;
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
    {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));

        // Simply get the state of the controller from XInput.
        dwResult = XInputGetState(i, &state);

        if (dwResult == ERROR_SUCCESS)
        {
            FControllers[i].State = state;
            FControllers[i].bIsConnected = true;
            ActiveControllerIndex = i;
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

float GamepadInputManager::GetMoveValueLX()
{
    return (FControllers[ActiveControllerIndex].MoveValueLX);
}

float GamepadInputManager::GetMoveValueRX()
{
    return (FControllers[ActiveControllerIndex].MoveValueRX);
}

