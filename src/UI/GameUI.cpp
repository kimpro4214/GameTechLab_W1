#include "UI/GameUI.h"

#include "Game/FruitCatalog.h"
#include "Game/GameConfig.h"
#include "Game/GameSession.h"
#include "ImGui/imgui.h"
#include "Rendering/FruitRenderer.h"

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
	DrawSceneOverlay(Session, Viewport);
	const EGameUICommand Command = Session.IsMainMenu()
		? DrawMainMenu(Viewport)
		: DrawGamePanel(Session, Viewport, InFruitRenderer);
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
	ImGui::SetNextWindowPos(
		ImVec2(Viewport.Width * 0.5f, Viewport.Height * 0.5f),
		ImGuiCond_Always,
		ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(360.0f, 220.0f), ImGuiCond_Always);
	ImGui::Begin(
		"Main Menu",
		nullptr,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove);

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::SetCursorPosX(100.0f);
	const bool bStartGame = ImGui::Button("Start Game", ImVec2(160.0f, 38.0f));
	ImGui::End();
	return bStartGame ? EGameUICommand::StartGame : EGameUICommand::None;
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

	if (Session.IsGameOver())
	{
		ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "GAME OVER");
		if (ImGui::Button("Main Menu"))
		{
			Command = EGameUICommand::ReturnToMainMenu;
		}
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
