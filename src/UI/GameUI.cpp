#include "pch.h"
#include "UI/GameUI.h"

#include "Audio/Audio.h"
#include "Game/FruitCatalog.h"
#include "Game/GameConfig.h"
#include "Game/GameSession.h"
#include "Game/Leaderboard.h"
#include "ImGui/imgui.h"
#include "Rendering/FruitRenderer.h"
#include <d3d11.h>

#include "Input/GamepadManager.h"

#include <cctype>
#include <cmath>

namespace
{
	constexpr float FruitPreviewSize = 40.0f;
	constexpr float FruitPreviewRadius = 18.0f;

	bool IsBlankName(const char* Name)
	{
		for (const char* Character = Name; *Character != '\0'; ++Character)
		{
			if (!std::isspace(static_cast<unsigned char>(*Character)))
			{
				return false;
			}
		}
		return true;
	}
}

EGameUICommand GameUI::Draw(
	const GameSession& Session,
	const D3D11_VIEWPORT& Viewport,
	const FruitRenderer& InFruitRenderer,
	Leaderboard& InLeaderboard,
	bool bShowLeaderboard)
{
	if (!Session.IsGameOver())
	{
		bHasSubmittedGameOverScore = false;
	}

	if (bShowLeaderboard)
	{
		return DrawLeaderboard(Viewport, InLeaderboard);
	}

	if (Session.IsMainMenu())
	{
		return DrawMainMenu(Viewport);
	}

	EGameUICommand Command = DrawGamePanel(Session, Viewport, InFruitRenderer);
	if (Session.IsGameOver())
	{
		Command = DrawGameOverPanel(Session, Viewport, InLeaderboard);
	}
	DrawCreatorCredit(Viewport);
	return Command;
}

EGameUICommand GameUI::DrawMainMenu(const D3D11_VIEWPORT& Viewport) const
{
	constexpr ImVec2 PanelSize(440.0f, 380.0f);
	constexpr ImVec2 ButtonSize(260.0f, 52.0f);
	constexpr char GameTitle[] = "FROGEGG GAME";

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
	ImGui::SetNextWindowFocus();
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
	if (ImGui::IsWindowAppearing())
	{
		ImGui::SetKeyboardFocusHere();
	}
	const bool bStartGame = ImGui::Button("START", ButtonSize);
	ImGui::SetCursorPosX((PanelSize.x - ButtonSize.x) * 0.5f);
	const bool bOpenLeaderboard = ImGui::Button("LEADERBOARD", ButtonSize);
	ImGui::SetCursorPosX((PanelSize.x - ButtonSize.x) * 0.5f);
	const bool bExitGame = ImGui::Button("EXIT", ButtonSize);
	ImGui::End();

	DrawCreatorCredit(Viewport);
	if (bStartGame)
	{
		Audio::GetInstance().Play("Click");
		return EGameUICommand::StartGame;
	}
	if (bOpenLeaderboard)
	{
		Audio::GetInstance().Play("Click");
		return EGameUICommand::OpenLeaderboard;
	}
	if (bExitGame)
	{
		Audio::GetInstance().Play("Click");
		return EGameUICommand::ExitGame;
	}
	return EGameUICommand::None;
}

EGameUICommand GameUI::DrawLeaderboard(
	const D3D11_VIEWPORT& Viewport,
	const Leaderboard& InLeaderboard) const
{
	constexpr ImVec2 PanelSize(460.0f, 440.0f);
	ImGui::SetNextWindowPos(
		ImVec2(Viewport.Width * 0.5f, Viewport.Height * 0.5f),
		ImGuiCond_Always,
		ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(PanelSize, ImGuiCond_Always);
	ImGui::Begin("Leaderboard", nullptr,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings);

	ImGui::SetWindowFontScale(1.8f);
	ImGui::Text("LEADERBOARD");
	ImGui::SetWindowFontScale(1.0f);
	ImGui::Separator();
	if (ImGui::BeginTable("LeaderboardTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("Rank", ImGuiTableColumnFlags_WidthFixed, 60.0f);
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Score", ImGuiTableColumnFlags_WidthFixed, 90.0f);
		ImGui::TableHeadersRow();
		const std::vector<LeaderboardEntry>& Entries = InLeaderboard.GetEntries();
		for (std::size_t Index = 0; Index < Entries.size(); ++Index)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%zu", Index + 1);
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(Entries[Index].Name.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%d", Entries[Index].Score);
		}
		ImGui::EndTable();
	}

	ImGui::SetCursorPosY(PanelSize.y - 65.0f);
	const bool bBack = ImGui::Button("Back", ImVec2(120.0f, 40.0f));
	ImGui::End();
	if (bBack)
	{
		Audio::GetInstance().Play("Click");
	}
	return bBack ? EGameUICommand::CloseLeaderboard : EGameUICommand::None;
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
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoFocusOnAppearing);

	ImGui::Text("Total Score : %d", Session.GetTotalScore());
	ImGui::SameLine();
	if (ImGui::Button("Restart"))
	{
		Audio::GetInstance().Play("Click");
		Command = EGameUICommand::RestartGame;
	}

	ImGui::Text("Next FrogEgg Color");
	DrawFruitPreview(Session.GetNextLevel());

	ImGui::Text("Storage FrogEgg Color (RightClick)");
	if (Session.GetStorageLevel() == -1)
	{
		ImGui::Text("Empty");
	}
	else
	{
		DrawFruitPreview(Session.GetStorageLevel());
	}

	ImGui::Text("FrogEgg Sequence");
	for (std::size_t i = 0; i < FruitCatalog::LevelCount; ++i)
	{
		DrawFruitPreview(static_cast<int>(i));
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
	const D3D11_VIEWPORT& Viewport,
	Leaderboard& InLeaderboard)
{
	constexpr ImVec2 PanelSize(420.0f, 360.0f);
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
	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted("Name");
	ImGui::SameLine();
	ImGui::SetCursorPosX(135.0f);
	ImGui::SetNextItemWidth(130.0f);
	ImGui::InputText("##Name", PlayerName, IM_ARRAYSIZE(PlayerName));
	ImGui::SameLine();
	ImGui::SetCursorPosX(275.0f);
	ImGui::BeginDisabled(bHasSubmittedGameOverScore || IsBlankName(PlayerName));
	if (ImGui::Button("Submit Score", ImVec2(120.0f, 0.0f)))
	{
		InLeaderboard.Add(PlayerName, Session.GetTotalScore());
		bHasSubmittedGameOverScore = true;
		Audio::GetInstance().Play("Click");
	}
	ImGui::EndDisabled();
	if (bHasSubmittedGameOverScore)
	{
		ImGui::TextDisabled("Score saved.");
	}

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
		Audio::GetInstance().Play("Click");
		return EGameUICommand::RestartGame;
	}
	if (bReturnToMainMenu)
	{
		Audio::GetInstance().Play("Click");
		return EGameUICommand::ReturnToMainMenu;
	}
	return EGameUICommand::None;
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
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing);
	ImGui::TextDisabled("Jaeho, Minkyu, Hyeongyu");
	ImGui::End();
}

void GameUI::DrawFruitPreview(int Level) const
{
	const ImVec2 PreviewSize(FruitPreviewSize, FruitPreviewSize);

	const FVector Color = FruitCatalog::GetColor(Level);

	const ImVec2 Position = ImGui::GetCursorScreenPos();
	const ImVec2 Center(
		Position.x + PreviewSize.x * 0.5f,
		Position.y + PreviewSize.y * 0.5f);
	ImDrawList* DrawList = ImGui::GetWindowDrawList();

	DrawList->AddCircleFilled(Center, 18.0f, ImColor(Color.x, Color.y, Color.z, 1.0f));
	DrawList->AddCircle(Center, 18.0f, IM_COL32(150, 230, 255, 210), 32, 2.0f);

	ImGui::Dummy(ImVec2(40.0f, 40.0f));
}

ImVec2 GameUI::ConvertWorldToScreen(
	const FVector& WorldLocation,
	const D3D11_VIEWPORT& Viewport) const
{
	const float WorldHeight = GameConfig::BottomBorder - GameConfig::TopBorder;
	return ImVec2(
		Viewport.TopLeftX + (WorldLocation.x + 1.0f) * 0.5f * Viewport.Width,
		Viewport.TopLeftY +
			(GameConfig::BottomBorder - WorldLocation.y) / WorldHeight * Viewport.Height);
}
