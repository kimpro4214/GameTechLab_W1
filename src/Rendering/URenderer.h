#pragma once

#include <wrl/client.h>
#include <d3d11.h>

#include <memory>

class RenderPipeline;
struct RenderPipelineDesc;

class Material;

class Mesh;
struct MeshDesc;

class URenderer
{
public:
	void Create(HWND hWindow);
	void Release();
	void InitImGui(HWND hWindow);
	void Prepare();
	void SwapBuffer();

	std::shared_ptr<RenderPipeline> CreateRenderPipeline(const RenderPipelineDesc& Desc);
	std::shared_ptr<Mesh> CreateMesh(const MeshDesc& Desc);
	Microsoft::WRL::ComPtr<ID3D11Buffer> CreateDynamicConstantBuffer(UINT ByteWidth);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTexture(LPCWSTR FilePath);
	Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateSamplerState(const D3D11_SAMPLER_DESC& Desc);

	template <typename T>
	void UpdateDynamicConstantBuffer(const Microsoft::WRL::ComPtr<ID3D11Buffer>& ConstantBuffer, const T& Constants);

	void Draw(const Material& Material, const Mesh& Mesh, ID3D11Buffer* ObjectConstantBuffer);

	D3D11_VIEWPORT ViewportInfo;									 // 렌더링 영역을 정의하는 뷰포트 정보

private:
	void CreateDeviceAndSwapChain(HWND hWindow);
	void CreateFrameBuffer();

	void ReleaseDeviceAndSwapChain();
	void ReleaseFrameBuffer();

	void BindMaterial(const Material& Material);
	void InvalidateStateCache();

private:
	// Direct3D 11 장치(Device)와 장치 컨텍스트(Device Context) 및 스왑 체인(Swap Chain)을 관리하기 위한 스마트 포인터들
	Microsoft::WRL::ComPtr<ID3D11Device> Device = nullptr; 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain = nullptr;

	// 렌더링에 필요한 리소스 및 상태를 관리하기 위한 변수들
	Microsoft::WRL::ComPtr<ID3D11Texture2D> FrameBuffer = nullptr;	   // 화면 출력용 텍스처
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> FrameBufferRTV = nullptr;  // 텍스처를 렌더 타겟으로 사용하는 뷰

	FLOAT		   ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f }; // 화면을 초기화(clear)할 때 사용할 색상 (RGBA)

	const RenderPipeline* BoundRenderPipeline = nullptr;
	const Material* BoundMaterial = nullptr;
};

template <typename T>
inline void URenderer::UpdateDynamicConstantBuffer(const Microsoft::WRL::ComPtr<ID3D11Buffer>& ConstantBuffer, const T& Constants)
{
	static_assert(std::is_trivially_copyable_v<T>);
	static_assert(sizeof(T) % 16 == 0);

	D3D11_MAPPED_SUBRESOURCE constantBufferMSR{};

	DeviceContext->Map(ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &constantBufferMSR); // update constant buffer every frame
	std::memcpy(constantBufferMSR.pData, &Constants, sizeof(T));
	DeviceContext->Unmap(ConstantBuffer.Get(), 0);
}
