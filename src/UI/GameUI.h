#pragma once

struct D3D11_VIEWPORT;
struct ID3D11ShaderResourceView;

class GameSession;
class FruitRenderer;
class Leaderboard;
struct FVector;
struct ImVec2;

enum class EGameUICommand
{
	None,
	StartGame,
	ExitGame,
	RestartGame,
	ReturnToMainMenu,
	OpenLeaderboard,
	CloseLeaderboard
};

class GameUI
{
public:
	EGameUICommand Draw(
		const GameSession& Session,
		const D3D11_VIEWPORT& Viewport,
		const FruitRenderer& InFruitRenderer,
		Leaderboard& InLeaderboard,
		bool bShowLeaderboard);

private:
	void DrawSceneOverlay(
		const GameSession& Session,
		const D3D11_VIEWPORT& Viewport) const;
	EGameUICommand DrawMainMenu(const D3D11_VIEWPORT& Viewport) const;
	EGameUICommand DrawLeaderboard(const D3D11_VIEWPORT& Viewport, const Leaderboard& InLeaderboard) const;
	EGameUICommand DrawGamePanel(
		const GameSession& Session,
		const D3D11_VIEWPORT& Viewport,
		const FruitRenderer& InFruitRenderer) const;
	EGameUICommand DrawGameOverPanel(
		const GameSession& Session,
		const D3D11_VIEWPORT& Viewport,
		Leaderboard& InLeaderboard);
	void DrawCreatorCredit(const D3D11_VIEWPORT& Viewport) const;
	void DrawFruitPreview(ID3D11ShaderResourceView* TextureSRV) const;
	ImVec2 ConvertWorldToScreen(
		const FVector& WorldLocation,
		const D3D11_VIEWPORT& Viewport) const;

	char PlayerName[17] = "";
	bool bHasSubmittedGameOverScore = false;
};
