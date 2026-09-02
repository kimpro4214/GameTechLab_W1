#include "UI/GameUI.h"

#include "Game/FruitCatalog.h"
#include "Game/GameConfig.h"
#include "Game/GameSession.h"
#include "ImGui/imgui.h"
#include "Rendering/FruitRenderer.h"
#include <d3d11.h>

#include <cmath>

namespace
{
	constexpr float FruitPreviewSize = 40.0f;
	constexpr float FruitPreviewRadius = 18.0f;
}

EGameUICommand GameUI::Draw(
	const GameSession& Session,
	const D3D11_VIEWPORT& Viewport,
	const FruitRenderer& InFruitRenderer) const
{
	if (Session.IsMainMenu())
	{
		return DrawMainMenu(Viewport);
	}

	DrawSceneOverlay(Session, Viewport);
	EGameUICommand Command = DrawGamePanel(Session, Viewport, InFruitRenderer);
	if (Session.IsGameOver())
	{
		Command = DrawGameOverPanel(Session, Viewport);
	}
	DrawCreatorCredit(Viewport);
	return Command;
}

void GameUI::DrawSceneOverlay(
	const GameSession& Session,
	const D3D11_VIEWPORT& Viewport) const
{
	ImDrawList* DrawList = ImGui::GetForegroundDrawList();
	if (!Session.IsMainMenu() && !Session.IsGameOver())
	{
		const UBall* CurrentBall = Session.GetCurrentBall();
		if (CurrentBall != nullptr && !CurrentBall->bHasBeenDropped)
		{
			const FVector GuideStart(
				CurrentBall->Location.x,
				CurrentBall->Location.y - CurrentBall->Radius,
				0.0f);
			const FVector GuideEnd(
				CurrentBall->Location.x,
				GameConfig::TopBorder + CurrentBall->Radius,
				0.0f);
			DrawList->AddLine(
				ConvertWorldToScreen(GuideStart, Viewport),
				ConvertWorldToScreen(GuideEnd, Viewport),
				IM_COL32(255, 255, 255, 90),
				2.0f);
		}
	}

	DrawList->AddRect(
		ConvertWorldToScreen(
			FVector(GameConfig::LeftBorder, GameConfig::TopBorder, 0.0f), Viewport),
		ConvertWorldToScreen(
			FVector(GameConfig::RightBorder, GameConfig::BottomBorder, 0.0f), Viewport),
		IM_COL32(255, 255, 255, 180),
		0.0f,
		0,
		2.0f);
	DrawList->AddLine(
		ConvertWorldToScreen(
			FVector(GameConfig::LeftBorder, GameConfig::GameOverLineY, 0.0f), Viewport),
		ConvertWorldToScreen(
			FVector(GameConfig::RightBorder, GameConfig::GameOverLineY, 0.0f), Viewport),
		IM_COL32(255, 80, 80, 255),
		2.0f);
}

EGameUICommand GameUI::DrawMainMenu(const D3D11_VIEWPORT& Viewport) const
{
	constexpr ImVec2 PanelSize(440.0f, 310.0f);
	constexpr ImVec2 ButtonSize(260.0f, 52.0f);
	constexpr char GameTitle[] = "WATERMELON GAME";

	ImDrawList* Background = ImGui::GetBackgroundDrawList();
	const ImVec2 ScreenMin(Viewport.TopLeftX, Viewport.TopLeftY);
	const ImVec2 ScreenMax(
		Viewport.TopLeftX + Viewport.Width,
		Viewport.TopLeftY + Viewport.Height);
	Background->AddRectFilled(ScreenMin, ScreenMax, IM_COL32(20, 28, 45, 255));
	Background->AddCircleFilled(
		ImVec2(Viewport.Width * 0.14f, Viewport.Height * 0.18f),
		Viewport.Height * 0.22f,
		IM_COL32(42, 72, 92, 120));
	Background->AddCircleFilled(
		ImVec2(Viewport.Width * 0.88f, Viewport.Height * 0.84f),
		Viewport.Height * 0.28f,
		IM_COL32(72, 48, 86, 100));

	ImGui::SetNextWindowPos(
		ImVec2(Viewport.Width * 0.5f, Viewport.Height * 0.44f),
		ImGuiCond_Always,
		ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(PanelSize, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin(
		"Title Screen",
		nullptr,
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings);

	ImGui::SetWindowFontScale(3.0f);
	const float TitleWidth = ImGui::CalcTextSize(GameTitle).x;
	ImGui::SetCursorPosX((PanelSize.x - TitleWidth) * 0.5f);
	ImGui::SetCursorPosY(18.0f);
	ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.24f, 1.0f), "%s", GameTitle);
	ImGui::SetWindowFontScale(1.0f);

	ImGui::SetCursorPosY(145.0f);
	ImGui::SetCursorPosX((PanelSize.x - ButtonSize.x) * 0.5f);
	const bool bStartGame = ImGui::Button("START", ButtonSize);
	ImGui::SetCursorPosX((PanelSize.x - ButtonSize.x) * 0.5f);
	const bool bExitGame = ImGui::Button("EXIT", ButtonSize);
	ImGui::End();

	DrawCreatorCredit(Viewport);
	if (bStartGame)
	{
		return EGameUICommand::StartGame;
	}
	return bExitGame ? EGameUICommand::ExitGame : EGameUICommand::None;
}

EGameUICommand GameUI::DrawGamePanel(
	const GameSession& Session,
	const D3D11_VIEWPORT& Viewport,
	const FruitRenderer& InFruitRenderer) const
{
	EGameUICommand Command = EGameUICommand::None;
	ImGui::SetNextWindowPos(ImVec2(Viewport.Width * 0.75f, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(
		ImVec2(Viewport.Width * 0.25f, Viewport.Height),
		ImGuiCond_Always);
	ImGui::Begin(
		"UI",
		nullptr,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove);

	ImGui::Text("Total Score : %d", Session.GetTotalScore());
	ImGui::SameLine();
	if (ImGui::Button("Restart"))
	{
		Command = EGameUICommand::RestartGame;
	}

	ImGui::Text("Next Fruit Color");
	DrawFruitPreview(InFruitRenderer.GetFruitTextureSRV(Session.GetNextLevel()));

	ImGui::Text("Storage Fruit Color (RightClick)");
	if (Session.GetStorageLevel() == -1)
	{
		ImGui::Text("Empty");
	}
	else
	{
		DrawFruitPreview(InFruitRenderer.GetFruitTextureSRV(Session.GetStorageLevel()));
	}

	ImGui::Text("Fruit Sequence");
	for (std::size_t i = 0; i < FruitCatalog::LevelCount; ++i)
	{
		DrawFruitPreview(InFruitRenderer.GetFruitTextureSRV(static_cast<int>(i)));
		const bool bHasNextFruit = i + 1 < FruitCatalog::LevelCount;
		const bool bIsEndOfRow = (i + 1) % 3 == 0;
		if (bHasNextFruit)
		{
			ImGui::SameLine();
			const float ArrowOffsetY =
				(FruitPreviewSize - ImGui::GetFrameHeight()) * 0.5f;
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ArrowOffsetY);
			ImGui::PushID(static_cast<int>(i));
			ImGui::ArrowButton("Right", ImGuiDir_Right);
			ImGui::PopID();
			if (!bIsEndOfRow)
			{
				ImGui::SameLine();
			}
		}
	}

	ImGui::End();
	return Command;
}

EGameUICommand GameUI::DrawGameOverPanel(
	const GameSession& Session,
	const D3D11_VIEWPORT& Viewport) const
{
	constexpr ImVec2 PanelSize(420.0f, 280.0f);
	constexpr ImVec2 RestartButtonSize(240.0f, 48.0f);

	ImGui::SetNextWindowPos(
		ImVec2(
			Viewport.TopLeftX + Viewport.Width * 0.5f,
			Viewport.TopLeftY + Viewport.Height * 0.5f),
		ImGuiCond_Always,
		ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(PanelSize, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.92f);
	ImGui::Begin(
		"Game Over",
		nullptr,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings);

	ImGui::SetWindowFontScale(2.5f);
	const char* GameOverText = "GAME OVER";
	const float TitleWidth = ImGui::CalcTextSize(GameOverText).x;
	ImGui::SetCursorPosX((PanelSize.x - TitleWidth) * 0.5f);
	ImGui::TextColored(ImVec4(1.0f, 0.08f, 0.08f, 1.0f), "%s", GameOverText);
	ImGui::SetWindowFontScale(1.0f);

	ImGui::Spacing();
	const char* ScoreText = "Final Score : %d";
	const ImVec2 ScoreSize = ImGui::CalcTextSize("Final Score : 000000");
	ImGui::SetCursorPosX((PanelSize.x - ScoreSize.x) * 0.5f);
	ImGui::Text(ScoreText, Session.GetTotalScore());

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::SetCursorPosX((PanelSize.x - RestartButtonSize.x) * 0.5f);
	const bool bRestart = ImGui::Button("Restart", RestartButtonSize);
	ImGui::Spacing();
	ImGui::SetCursorPosX((PanelSize.x - RestartButtonSize.x) * 0.5f);
	const bool bReturnToMainMenu = ImGui::Button("Main Menu", RestartButtonSize);
	ImGui::End();

	if (bRestart)
	{
		return EGameUICommand::RestartGame;
	}
	return bReturnToMainMenu
		? EGameUICommand::ReturnToMainMenu
		: EGameUICommand::None;
}

void GameUI::DrawCreatorCredit(const D3D11_VIEWPORT& Viewport) const
{
	ImGui::SetNextWindowPos(
		ImVec2(Viewport.Width - 12.0f, Viewport.Height - 12.0f),
		ImGuiCond_Always,
		ImVec2(1.0f, 1.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin(
		"Creator Credit",
		nullptr,
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings);
	ImGui::TextDisabled("Jaeho, Minkyu, Hyeongyu");
	ImGui::End();
}

void GameUI::DrawFruitPreview(ID3D11ShaderResourceView* TextureSRV) const
{
	const ImVec2 PreviewSize(FruitPreviewSize, FruitPreviewSize);

	if (!TextureSRV)
	{
		ImGui::Dummy(PreviewSize);
		return;
	}

	const ImVec2 PreviewPosition = ImGui::GetCursorScreenPos();
	const ImVec2 PreviewCenter(
		PreviewPosition.x + PreviewSize.x * 0.5f,
		PreviewPosition.y + PreviewSize.y * 0.5f);
	ImDrawList* DrawList = ImGui::GetWindowDrawList();

	ImGui::Image(reinterpret_cast<ImTextureID>(TextureSRV), PreviewSize);
}

ImVec2 GameUI::ConvertWorldToScreen(
	const FVector& WorldLocation,
	const D3D11_VIEWPORT& Viewport) const
{
	return ImVec2(
		Viewport.TopLeftX + (WorldLocation.x + 1.0f) * 0.5f * Viewport.Width,
		Viewport.TopLeftY + (1.0f - WorldLocation.y) * 0.5f * Viewport.Height);
}
