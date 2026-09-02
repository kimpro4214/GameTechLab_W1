#include "pch.h"
#include "App/GameApplication.h"

#include "Game/GameConfig.h"
#include "Game/GameController.h"
#include "Game/GameInput.h"
#include "Game/GameSession.h"
#include "Game/Leaderboard.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "Platform/Win32Window.h"
#include "Rendering/URenderer.h"
#include "Rendering/FruitRenderer.h"
#include "Rendering/ParticleRenderer.h"
#include "UI/GameUI.h"
#include "Audio/Audio.h"

#include "Input/GamepadInputManager.h"

namespace
{
	class ScopedComInitialization
	{
	public:
		ScopedComInitialization()
			: Result(CoInitializeEx(nullptr, COINIT_MULTITHREADED))
		{
		}

		~ScopedComInitialization()
		{
			if (SUCCEEDED(Result))
			{
				CoUninitialize();
			}
		}

		bool IsAvailable() const
		{
			return SUCCEEDED(Result) || Result == RPC_E_CHANGED_MODE;
		}

	private:
		HRESULT Result;
	};

	FGameInput BuildGameInput(
		const ImGuiIO& FrameIO,
		const D3D11_VIEWPORT& Viewport)
	{
		FGameInput Input;
		if (Viewport.Width <= 0.0f || Viewport.Height <= 0.0f)
		{
			return Input;
		}

		const ImVec2 MousePosition = FrameIO.MousePos;
		const float MouseWorldX =
			((MousePosition.x - Viewport.TopLeftX) / Viewport.Width) * 2.0f - 1.0f;
		const float MouseWorldY =
			1.0f - ((MousePosition.y - Viewport.TopLeftY) / Viewport.Height) * 2.0f;
		const bool bIsInViewport =
			MousePosition.x >= Viewport.TopLeftX &&
			MousePosition.x <= Viewport.TopLeftX + Viewport.Width &&
			MousePosition.y >= Viewport.TopLeftY &&
			MousePosition.y <= Viewport.TopLeftY + Viewport.Height;
		const bool bIsInGameBounds =
			bIsInViewport &&
			MouseWorldX >= GameConfig::LeftBorder &&
			MouseWorldX <= GameConfig::RightBorder &&
			MouseWorldY >= GameConfig::TopBorder &&
			MouseWorldY <= GameConfig::BottomBorder;

		Input.MouseWorldX = MouseWorldX;
		Input.bCanUseSceneMouse = bIsInGameBounds && !FrameIO.WantCaptureMouse;
		Input.bIsLeftMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
		Input.bIsLeftMouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
		Input.bIsRightMouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Right);

		GamepadManager::GetInstance().Update();
		return Input;
	}

	void ApplyUICommand(
		EGameUICommand Command,
		GameSession& Session,
		GameController& Controller,
		bool& bShowLeaderboard)
	{
		switch (Command)
		{
		case EGameUICommand::StartGame:
			Session.StartGame();
			Controller.Reset();
			break;
		case EGameUICommand::ExitGame:
			PostQuitMessage(0);
			break;
		case EGameUICommand::RestartGame:
			Session.RestartGame();
			Controller.Reset();
			break;
		case EGameUICommand::ReturnToMainMenu:
			Session.ReturnToMainMenu();
			Controller.Reset();
			break;
		case EGameUICommand::OpenLeaderboard:
			bShowLeaderboard = true;
			break;
		case EGameUICommand::CloseLeaderboard:
			bShowLeaderboard = false;
			break;
		case EGameUICommand::None:
		default:
			break;
		}
	}
}

int GameApplication::Run(HINSTANCE Instance, int ShowCommand)
{
	ScopedComInitialization ComInitialization;
	if (!ComInitialization.IsAvailable())
	{
		return 1;
	}

	Win32Window Window;
	if (!Window.Create(Instance, ShowCommand))
	{
		return 2;
	}

	URenderer Renderer;
	Renderer.Create(Window.GetHandle());
	Renderer.InitImGui(Window.GetHandle());

	FruitRenderer FruitSceneRenderer;
	if (!FruitSceneRenderer.Initialize(Renderer))
	{
		FruitSceneRenderer.Release();
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		Renderer.Release();
		return 3;
	}

	ParticleRenderer MergeParticleRenderer;
	if (!MergeParticleRenderer.Initialize(Renderer))
	{
		FruitSceneRenderer.Release();
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		Renderer.Release();
		return 3;
	}

	if (!Audio::GetInstance().Initialize(Window.GetHandle()))
	{
		return 4;
	}

	GamepadManager::GetInstance().Initialize();

	GameSession Session;
	GameController Controller;
	GameUI UI;
	Leaderboard Scoreboard;
	Scoreboard.Load();
	bool bShowLeaderboard = false;

	constexpr float FrameDeltaTime =
		1.0f / static_cast<float>(GameConfig::TargetFps);
	constexpr double TargetFrameTimeMilliseconds =
		1000.0 / GameConfig::TargetFps;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);

	

	bool bShouldExit = false;
	while (!bShouldExit)
	{
		LARGE_INTEGER FrameStart;
		QueryPerformanceCounter(&FrameStart);

		MSG Message;
		while (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
			if (Message.message == WM_QUIT)
			{
				bShouldExit = true;
				break;
			}
		}
		if (bShouldExit)
		{
			break;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();

		Controller.HandleInput(
			Session,
			BuildGameInput(ImGui::GetIO(), Renderer.ViewportInfo));
		Session.Update(FrameDeltaTime);

		Renderer.Prepare();
		if (!Session.IsMainMenu())
		{
			FruitSceneRenderer.Draw(Renderer, Session.GetBalls());
			MergeParticleRenderer.Draw(Renderer, Session.GetParticles());
		}
		ApplyUICommand(
			UI.Draw(Session, Renderer.ViewportInfo, FruitSceneRenderer, Scoreboard, bShowLeaderboard),
			Session,
			Controller,
			bShowLeaderboard);

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		Renderer.SwapBuffer();

		double ElapsedMilliseconds = 0.0;
		do
		{
			Sleep(0);
			LARGE_INTEGER FrameEnd;
			QueryPerformanceCounter(&FrameEnd);
			ElapsedMilliseconds =
				(FrameEnd.QuadPart - FrameStart.QuadPart) * 1000.0 /
				Frequency.QuadPart;
		} while (ElapsedMilliseconds < TargetFrameTimeMilliseconds);
	}

	Audio::GetInstance().Shutdown();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	MergeParticleRenderer.Release();
	FruitSceneRenderer.Release();
	Renderer.Release();
	return 0;
}
