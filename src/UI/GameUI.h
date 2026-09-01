#pragma once

#include <d3d11.h>

class GameSession;
class FruitRenderer;
struct FVector;
struct ImVec2;

enum class EGameUICommand
{
	None,
	StartGame,
	RestartGame,
	ReturnToMainMenu
};

class GameUI
{
public:
	EGameUICommand Draw(
		const GameSession& Session,
		const D3D11_VIEWPORT& Viewport,
		const FruitRenderer& InFruitRenderer) const;

private:
	void DrawSceneOverlay(
		const GameSession& Session,
		const D3D11_VIEWPORT& Viewport) const;
	EGameUICommand DrawMainMenu(const D3D11_VIEWPORT& Viewport) const;
	EGameUICommand DrawGamePanel(
		const GameSession& Session,
		const D3D11_VIEWPORT& Viewport,
		const FruitRenderer& InFruitRenderer) const;
	void DrawCreatorCredit(const D3D11_VIEWPORT& Viewport) const;
	void DrawFruitPreview(ID3D11ShaderResourceView* TextureSRV) const;
	ImVec2 ConvertWorldToScreen(
		const FVector& WorldLocation,
		const D3D11_VIEWPORT& Viewport) const;
};
