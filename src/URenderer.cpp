#include "URenderer.h"

#include <wrl/client.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "Material.h"
#include "RenderPipeline.h"
#include "Mesh.h"

void URenderer::Create(HWND hWindow)
{
	// Direct3D 장치 및 스왑 체인 생성
	CreateDeviceAndSwapChain(hWindow);

	// 프레임 버퍼 생성
	CreateFrameBuffer();

	// 깊이 스텐실 버퍼 및 블렌드 상태는 이 코드에서는 다루지 않음
}

// 렌더러에 사용된 모든 리소스를 해제하는 함수
void URenderer::Release()
{
	// 렌더 타겟을 초기화
	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	ReleaseFrameBuffer();
	ReleaseDeviceAndSwapChain();
}


void URenderer::InitImGui(HWND hWindow)
{
	// 여기에서 ImGui를 생성합니다.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWindow);
	ImGui_ImplDX11_Init(Device, DeviceContext);
}

void URenderer::Prepare()
{
	DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);

	DeviceContext->RSSetViewports(1, &ViewportInfo);

	DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, nullptr);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	InvalidateStateCache();
}

// 스왑 체인의 백 버퍼와 프론트 버퍼를 교체하여 화면에 출력
void URenderer::SwapBuffer()
{
	SwapChain->Present(1, 0); // 1: VSync 활성화
}

std::shared_ptr<RenderPipeline> URenderer::CreateRenderPipeline(const RenderPipelineDesc& Desc)
{
	if (!Desc.InputElements ||
		Desc.InputElementCount == 0)
	{
		return nullptr;
	}

	auto Result = std::make_shared<RenderPipeline>();

	Microsoft::WRL::ComPtr<ID3DBlob> VertexShaderBlob;
	HRESULT hr = D3DCompileFromFile(Desc.ShaderFileName, nullptr, nullptr, Desc.VertexEntryPoint, "vs_5_0", 0, 0, VertexShaderBlob.GetAddressOf(), nullptr);
	if (FAILED(hr))
	{
		return nullptr;
	}
	hr = Device->CreateVertexShader(VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), nullptr, Result->VertexShader.GetAddressOf());
	if (FAILED(hr))
	{
		return nullptr;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> PixelShaderBlob;
	hr = D3DCompileFromFile(Desc.ShaderFileName, nullptr, nullptr, Desc.PixelEntryPoint, "ps_5_0", 0, 0, PixelShaderBlob.GetAddressOf(), nullptr);
	if (FAILED(hr))
	{
		return nullptr;
	}
	hr = Device->CreatePixelShader(PixelShaderBlob->GetBufferPointer(), PixelShaderBlob->GetBufferSize(), nullptr, Result->PixelShader.GetAddressOf());
	if (FAILED(hr))
	{
		return nullptr;
	}

	hr = Device->CreateInputLayout(Desc.InputElements, Desc.InputElementCount, VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), Result->InputLayout.GetAddressOf());
	if (FAILED(hr))
	{
		return nullptr;
	}

	D3D11_RASTERIZER_DESC Rasterizerdesc = {};
	Rasterizerdesc.FillMode = D3D11_FILL_SOLID;
	Rasterizerdesc.CullMode = D3D11_CULL_BACK;

	hr = Device->CreateRasterizerState(&Rasterizerdesc, Result->RasterizerState.GetAddressOf());
	if (FAILED(hr))
	{
		return nullptr;
	}

	return Result;
}

std::shared_ptr<Mesh> URenderer::CreateMesh(const MeshDesc& Desc)
{
	if (!Desc.VertexData ||
		Desc.VertexDataSize == 0 ||
		Desc.VertexStride == 0 ||
		Desc.VertexCount == 0)
	{
		return nullptr;
	}

	if (Desc.IndexData && Desc.IndexCount > 0 && Desc.IndexDataSize == 0)
	{
		return nullptr;
	}

	auto ResultMesh = std::make_shared<Mesh>();

	D3D11_BUFFER_DESC VertexBufferDesc = {};
	VertexBufferDesc.ByteWidth = Desc.VertexDataSize;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA VertexInitialData = {};
	VertexInitialData.pSysMem = Desc.VertexData;

	HRESULT hr = Device->CreateBuffer(&VertexBufferDesc, &VertexInitialData, ResultMesh->VertexBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		return nullptr;
	}

	if (Desc.IndexData && Desc.IndexCount > 0)
	{
		D3D11_BUFFER_DESC IndexBufferDesc = {};
		IndexBufferDesc.ByteWidth = Desc.IndexDataSize;
		IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA IndexInitialData = {};
		IndexInitialData.pSysMem = Desc.IndexData;

		hr = Device->CreateBuffer(&IndexBufferDesc, &IndexInitialData, ResultMesh->IndexBuffer.GetAddressOf());
		if (FAILED(hr))
		{
			return nullptr;
		}
	}

	ResultMesh->VertexStride = Desc.VertexStride;
	ResultMesh->VertexCount = Desc.VertexCount;
	ResultMesh->IndexCount = Desc.IndexCount;
	ResultMesh->IndexFormat = Desc.IndexFormat;
	ResultMesh->Topology = Desc.Topology;

	return ResultMesh;
}

Microsoft::WRL::ComPtr<ID3D11Buffer> URenderer::CreateDynamicConstantBuffer(UINT ByteWidth)
{
	if (ByteWidth == 0 || ByteWidth % 16 != 0)
	{
		return nullptr;
	}

	D3D11_BUFFER_DESC Desc{};
	Desc.ByteWidth = ByteWidth;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	Microsoft::WRL::ComPtr<ID3D11Buffer> Result;

	HRESULT hr = Device->CreateBuffer(&Desc, nullptr, Result.GetAddressOf());

	return SUCCEEDED(hr) ? Result : nullptr;
}

void URenderer::Draw(const Material& Material, const Mesh& Mesh, ID3D11Buffer* ObjectConstantBuffer)
{
	BindMaterial(Material);
	Mesh.Bind(DeviceContext);

	DeviceContext->VSSetConstantBuffers(0, 1, &ObjectConstantBuffer);
	
	if (Mesh.HasIndices())
	{
		DeviceContext->DrawIndexed(Mesh.GetIndexCount(), 0, 0);
	}
	else
	{
		DeviceContext->Draw(Mesh.GetVertexCount(), 0);
	}
}

// Direct3D 장치 및 스왑 체인을 생성하는 함수
void URenderer::CreateDeviceAndSwapChain(HWND hWindow)
{
	// 지원하는 Direct3D 기능 레벨을 정의
	D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

	// 스왑 체인 설정 구조체 초기화
	DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
	swapchaindesc.BufferDesc.Width = 0;							  // 창 크기에 맞게 자동으로 설정
	swapchaindesc.BufferDesc.Height = 0;						  // 창 크기에 맞게 자동으로 설정
	swapchaindesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 색상 포맷
	swapchaindesc.SampleDesc.Count = 1;							  // 멀티 샘플링 비활성화
	swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // 렌더 타겟으로 사용
	swapchaindesc.BufferCount = 2;								  // 더블 버퍼링
	swapchaindesc.OutputWindow = hWindow;						  // 렌더링할 창 핸들
	swapchaindesc.Windowed = TRUE;								  // 창 모드
	swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	  // 스왑 방식

	// Direct3D 장치와 스왑 체인을 생성
	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
		featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
		&swapchaindesc, &SwapChain, &Device, nullptr, &DeviceContext);

	// 생성된 스왑 체인의 정보 가져오기
	SwapChain->GetDesc(&swapchaindesc);

	// 뷰포트 정보 설정
	ViewportInfo = { 0.0f, 0.0f, (float)swapchaindesc.BufferDesc.Width, (float)swapchaindesc.BufferDesc.Height, 0.0f, 1.0f };
}

// 프레임 버퍼를 생성하는 함수
void URenderer::CreateFrameBuffer()
{
	// 스왑 체인으로부터 백 버퍼 텍스처 가져오기
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

	// 렌더 타겟 뷰 생성
	D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
	framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;	  // 색상 포맷
	framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

	Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
}

// Direct3D 장치 및 스왑 체인을 해제하는 함수
void URenderer::ReleaseDeviceAndSwapChain()
{
	if (DeviceContext)
	{
		DeviceContext->Flush(); // 남아있는 GPU 명령 실행
	}

	if (SwapChain)
	{
		SwapChain->Release();
		SwapChain = nullptr;
	}

	if (DeviceContext)
	{
		DeviceContext->Release();
		DeviceContext = nullptr;
	}

	if (Device)
	{
		Device->Release();
		Device = nullptr;
	}
}

// 프레임 버퍼를 해제하는 함수
void URenderer::ReleaseFrameBuffer()
{
	if (FrameBufferRTV)
	{
		FrameBufferRTV->Release();
		FrameBufferRTV = nullptr;
	}

	if (FrameBuffer)
	{
		FrameBuffer->Release();
		FrameBuffer = nullptr;
	}
}

void URenderer::BindMaterial(const Material& Material)
{
	const RenderPipeline& Pipeline = Material.GetRendererPipeline();

	if (BoundRenderPipeline != &Pipeline)
	{
		Pipeline.Bind(DeviceContext);
		BoundRenderPipeline = &Pipeline;

		BoundMaterial = nullptr;
	}

	if (BoundMaterial != &Material)
	{
		Material.BindResources(DeviceContext);
		BoundMaterial = &Material;
	}
}

void URenderer::InvalidateStateCache()
{
	BoundRenderPipeline = nullptr;
	BoundMaterial = nullptr;
}
