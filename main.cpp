#include <windows.h>

// D3D 사용에 필요한 라이브러리들을 링크합니다.
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// D3D 사용에 필요한 헤더파일들을 포함합니다.
#include <d3d11.h>
#include <d3dcompiler.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"

// 1. Define the triangle vertices
struct FVertexSimple
{
	float x, y, z;	  // Position
	float r, g, b, a; // Color
};

// Structure for a 3D vector
struct FVector
{
	float x, y, z;
	FVector(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

	FVector operator+(const FVector& Other) const
	{
		return FVector(x + Other.x, y + Other.y, z + Other.z);
	}

	FVector operator-(const FVector& Other) const
	{
		return FVector(x - Other.x, y - Other.y, z - Other.z);
	}

	FVector operator*(float Scalar) const
	{
		return FVector(x * Scalar, y * Scalar, z * Scalar);
	}

	FVector operator/(float Scalar) const
	{
		return FVector(x / Scalar, y / Scalar, z / Scalar);
	}

	FVector& operator+=(const FVector& Other)
	{
		x += Other.x;
		y += Other.y;
		z += Other.z;
		return *this;
	}

	FVector& operator-=(const FVector& Other)
	{
		x -= Other.x;
		y -= Other.y;
		z -= Other.z;
		return *this;
	}

	FVector& operator*=(float Scalar)
	{
		x *= Scalar;
		y *= Scalar;
		z *= Scalar;
		return *this;
	}

	static float DotProduct(const FVector& A, const FVector& B)
	{
		return A.x * B.x + A.y * B.y + A.z * B.z;
	}
};

#include "Sphere.h"

// 삼각형을 하드 코딩
FVertexSimple triangle_vertices[] = {
	{ 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f },  // Top vertex (red)
	{ 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right vertex (green)
	{ -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f } // Bottom-left vertex (blue)
};

FVertexSimple cube_vertices[] = {
	// Front face (Z+)
	{ -0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f }, // Bottom-left (red)
	{ -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f },	// Top-left (yellow)
	{ 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f },	// Bottom-right (green)
	{ -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f },	// Top-left (yellow)
	{ 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },	// Top-right (blue)
	{ 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f },	// Bottom-right (green)

	// Back face (Z-)
	{ -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f }, // Bottom-left (cyan)
	{ 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f },	 // Bottom-right (magenta)
	{ -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f },	 // Top-left (blue)
	{ -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f },	 // Top-left (blue)
	{ 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f },	 // Bottom-right (magenta)
	{ 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f },	 // Top-right (yellow)

	// Left face (X-)
	{ -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f }, // Bottom-left (purple)
	{ -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f },	 // Top-left (blue)
	{ -0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f },	 // Bottom-right (green)
	{ -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f },	 // Top-left (blue)
	{ -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f },	 // Top-right (yellow)
	{ -0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f },	 // Bottom-right (green)

	// Right face (X+)
	{ 0.5f, -0.5f, -0.5f, 1.0f, 0.5f, 0.0f, 1.0f }, // Bottom-left (orange)
	{ 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 1.0f },	// Bottom-right (gray)
	{ 0.5f, 0.5f, -0.5f, 0.5f, 0.0f, 0.5f, 1.0f },	// Top-left (purple)
	{ 0.5f, 0.5f, -0.5f, 0.5f, 0.0f, 0.5f, 1.0f },	// Top-left (purple)
	{ 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 1.0f },	// Bottom-right (gray)
	{ 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.5f, 1.0f },	// Top-right (dark blue)

	// Top face (Y+)
	{ -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.5f, 1.0f }, // Bottom-left (light green)
	{ -0.5f, 0.5f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f },	// Top-left (cyan)
	{ 0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f },	// Bottom-right (white)
	{ -0.5f, 0.5f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f },	// Top-left (cyan)
	{ 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 1.0f },	// Top-right (brown)
	{ 0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f },	// Bottom-right (white)

	// Bottom face (Y-)
	{ -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.0f, 1.0f }, // Bottom-left (brown)
	{ -0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f },	 // Top-left (red)
	{ 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.5f, 1.0f },	 // Bottom-right (purple)
	{ -0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f },	 // Top-left (red)
	{ 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f },	 // Top-right (green)
	{ 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.5f, 1.0f },	 // Bottom-right (purple)
};

class URenderer
{
public:
	// Direct3D 11 장치(Device)와 장치 컨텍스트(Device Context) 및 스왑 체인(Swap Chain)을 관리하기 위한 포인터들
	ID3D11Device*		 Device = nullptr;		  // GPU와 통신하기 위한 Direct3D 장치
	ID3D11DeviceContext* DeviceContext = nullptr; // GPU 명령 실행을 담당하는 컨텍스트
	IDXGISwapChain*		 SwapChain = nullptr;	  // 프레임 버퍼를 교체하는 데 사용되는 스왑 체인

	// 렌더링에 필요한 리소스 및 상태를 관리하기 위한 변수들
	ID3D11Texture2D*		FrameBuffer = nullptr;	   // 화면 출력용 텍스처
	ID3D11RenderTargetView* FrameBufferRTV = nullptr;  // 텍스처를 렌더 타겟으로 사용하는 뷰
	ID3D11RasterizerState*	RasterizerState = nullptr; // 래스터라이저 상태(컬링, 채우기 모드 등 정의)
	ID3D11Buffer*			ConstantBuffer = nullptr;  // 쉐이더에 데이터를 전달하기 위한 상수 버퍼

	FLOAT		   ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f }; // 화면을 초기화(clear)할 때 사용할 색상 (RGBA)
	D3D11_VIEWPORT ViewportInfo;									 // 렌더링 영역을 정의하는 뷰포트 정보

public:
	// 렌더러 초기화 함수
	void Create(HWND hWindow)
	{
		// Direct3D 장치 및 스왑 체인 생성
		CreateDeviceAndSwapChain(hWindow);

		// 프레임 버퍼 생성
		CreateFrameBuffer();

		// 래스터라이저 상태 생성
		CreateRasterizerState();

		// 깊이 스텐실 버퍼 및 블렌드 상태는 이 코드에서는 다루지 않음
	}

	// Direct3D 장치 및 스왑 체인을 생성하는 함수
	void CreateDeviceAndSwapChain(HWND hWindow)
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

	// Direct3D 장치 및 스왑 체인을 해제하는 함수
	void ReleaseDeviceAndSwapChain()
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

	// 프레임 버퍼를 생성하는 함수
	void CreateFrameBuffer()
	{
		// 스왑 체인으로부터 백 버퍼 텍스처 가져오기
		SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

		// 렌더 타겟 뷰 생성
		D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
		framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;	  // 색상 포맷
		framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

		Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
	}

	// 프레임 버퍼를 해제하는 함수
	void ReleaseFrameBuffer()
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

	// 래스터라이저 상태를 생성하는 함수
	void CreateRasterizerState()
	{
		D3D11_RASTERIZER_DESC rasterizerdesc = {};
		rasterizerdesc.FillMode = D3D11_FILL_SOLID; // 채우기 모드
		rasterizerdesc.CullMode = D3D11_CULL_BACK;	// 백 페이스 컬링

		Device->CreateRasterizerState(&rasterizerdesc, &RasterizerState);
	}

	// 래스터라이저 상태를 해제하는 함수
	void ReleaseRasterizerState()
	{
		if (RasterizerState)
		{
			RasterizerState->Release();
			RasterizerState = nullptr;
		}
	}

	// 렌더러에 사용된 모든 리소스를 해제하는 함수
	void Release()
	{
		RasterizerState->Release();

		// 렌더 타겟을 초기화
		DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

		ReleaseFrameBuffer();
		ReleaseDeviceAndSwapChain();
	}

	// 스왑 체인의 백 버퍼와 프론트 버퍼를 교체하여 화면에 출력
	void SwapBuffer()
	{
		SwapChain->Present(1, 0); // 1: VSync 활성화
	}

public:
	ID3D11VertexShader* SimpleVertexShader;
	ID3D11PixelShader*	SimplePixelShader;
	ID3D11InputLayout*	SimpleInputLayout;
	unsigned int		Stride;

	// 두 종류의 셰이더(VS, PS)를 컴파일해서 준비하고, 들어올 데이터의 규격(Layout)을 정의하는 함수
	void CreateShader()
	{
		ID3DBlob* vertexshaderCSO;
		ID3DBlob* pixelshaderCSO;

		// vs 컴파일
		D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vertexshaderCSO, nullptr);

		Device->CreateVertexShader(vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), nullptr, &SimpleVertexShader);
		// ps 컴파일
		D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &pixelshaderCSO, nullptr);

		Device->CreatePixelShader(pixelshaderCSO->GetBufferPointer(), pixelshaderCSO->GetBufferSize(), nullptr, &SimplePixelShader);
		// layout 정의
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		Device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), &SimpleInputLayout);

		Stride = sizeof(FVertexSimple);

		vertexshaderCSO->Release();
		pixelshaderCSO->Release();
	}

	void ReleaseShader()
	{
		if (SimpleInputLayout)
		{
			SimpleInputLayout->Release();
			SimpleInputLayout = nullptr;
		}

		if (SimplePixelShader)
		{
			SimplePixelShader->Release();
			SimplePixelShader = nullptr;
		}

		if (SimpleVertexShader)
		{
			SimpleVertexShader->Release();
			SimpleVertexShader = nullptr;
		}
	}

public:
	void Prepare()
	{
		DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);

		DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		DeviceContext->RSSetViewports(1, &ViewportInfo);
		DeviceContext->RSSetState(RasterizerState);

		DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, nullptr);
		DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	}

	void PrepareShader()
	{
		DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
		DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);
		DeviceContext->IASetInputLayout(SimpleInputLayout);

		// 버텍스 쉐이더에 상수 버퍼를 설정합니다.
		if (ConstantBuffer)
		{
			DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
		}
	}

	void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices)
	{
		UINT offset = 0;
		DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);

		DeviceContext->Draw(numVertices, 0);
	}

	ID3D11Buffer* CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth)
	{
		// 2. Create a vertex buffer
		D3D11_BUFFER_DESC vertexbufferdesc = {};
		vertexbufferdesc.ByteWidth = byteWidth;
		vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated
		vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexbufferSRD = { vertices };

		ID3D11Buffer* vertexBuffer;

		Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);

		return vertexBuffer;
	}

	void ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer)
	{
		vertexBuffer->Release();
	}

	struct FConstants
	{
		FVector Offset;
		float	Scale;
	};

	void CreateConstantBuffer()
	{
		D3D11_BUFFER_DESC constantbufferdesc = {};
		constantbufferdesc.ByteWidth = sizeof(FConstants) + 0xf & 0xfffffff0; // ensure constant buffer size is multiple of 16 bytes
		constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC;						  // will be updated from CPU every frame
		constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		Device->CreateBuffer(&constantbufferdesc, nullptr, &ConstantBuffer);
	}

	void ReleaseConstantBuffer()
	{
		if (ConstantBuffer)
		{
			ConstantBuffer->Release();
			ConstantBuffer = nullptr;
		}
	}

	void UpdateConstant(const FVector& Offset, float Scale)
	{
		if (ConstantBuffer)
		{
			D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

			DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR); // update constant buffer every frame
			FConstants* constants = (FConstants*)constantbufferMSR.pData;
			{
				constants->Offset = Offset;
				constants->Scale = Scale;
			}
			DeviceContext->Unmap(ConstantBuffer, 0);
		}
	}
};

class UPrimitive
{
public:
	virtual ~UPrimitive() = default;
	virtual bool IsColliding(const UPrimitive* Other) const = 0;
	virtual void ResolveCollision(UPrimitive* Other) = 0;
	virtual void AddVelocity(const FVector& DeltaVelocity) = 0;
};

class UBall : public UPrimitive
{
public:
	FVector	   Location;
	FVector	   Velocity;
	float	   Radius;
	float	   Mass;
	static int TotalNumBalls;

	UBall(const FVector& InitialLocation = FVector(), const FVector& InitialVelocity = FVector(), float InitialRadius = 0.1f)
		: Location(InitialLocation), Velocity(InitialVelocity), Radius(0.0f), Mass(0.0f)
	{
		SetRadius(InitialRadius);
		++TotalNumBalls;
	}

	~UBall() override
	{
		--TotalNumBalls;
	}
	// 복사가 필요 없는 타입이라고 생각하여 이펙티브 모던 C++에서 배운 =delete를 사용해 복사 생성과 복사 대입을 금지
	UBall(const UBall&) = delete;
	UBall& operator=(const UBall&) = delete;

	bool IsColliding(const UPrimitive* OtherPrimitive) const override
	{
		const UBall* OtherBall = dynamic_cast<const UBall*>(OtherPrimitive);
		if (OtherBall == nullptr || OtherBall == this)
		{
			return false;
		}

		const FVector Delta = OtherBall->Location - Location;
		const float	  RadiusSum = Radius + OtherBall->Radius;

		return FVector::DotProduct(Delta, Delta) <= RadiusSum * RadiusSum;
	}

	void ResolveCollision(UPrimitive* Other) override
	{
		UBall* OtherBall = dynamic_cast<UBall*>(Other);
		if (OtherBall == nullptr || OtherBall == this)
		{
			return;
		}

		const FVector Delta = OtherBall->Location - Location;
		const float	  RadiusSum = Radius + OtherBall->Radius;
		const float	  DistanceSquared = FVector::DotProduct(Delta, Delta);

		if (DistanceSquared > RadiusSum * RadiusSum)
		{
			return;
		}

		float	Distance = 0.0f;
		FVector CollisionNormal(1.0f, 0.0f, 0.0f);

		if (DistanceSquared > 0.000001f)
		{
			Distance = sqrtf(DistanceSquared);
			CollisionNormal = Delta / Distance;
		}

		const float InverseMass = 1.0f / Mass;
		const float OtherInverseMass = 1.0f / OtherBall->Mass;
		const float InverseMassSum = InverseMass + OtherInverseMass;

		// 겹친 거리를 질량 비율에 따라 나누어 두 공을 서로 밀어냄
		const float	  Penetration = RadiusSum - Distance;
		const FVector Correction = CollisionNormal * (Penetration / InverseMassSum);
		Location -= Correction * InverseMass;
		OtherBall->Location += Correction * OtherInverseMass;

		const FVector RelativeVelocity = OtherBall->Velocity - Velocity;
		const float	  VelocityAlongNormal = FVector::DotProduct(RelativeVelocity, CollisionNormal);

		// 이미 서로 멀어지는 중이라면 위치만 보정하고 추가 충격량은 적용 X
		if (VelocityAlongNormal >= 0.0f)
		{
			return;
		}

		// 공기 저항 없고, 마찰 없고, 공 끼리 탄성 충돌을 하기에 반발계수를 1로 설정
		// 반발계수 1인 완전 탄성 충돌의 충격량을 계산
		const float	  Impulse = -(1.0f + 1.0f) * VelocityAlongNormal / InverseMassSum;
		const FVector ImpulseVector = CollisionNormal * Impulse;
		Velocity -= ImpulseVector * InverseMass;
		OtherBall->Velocity += ImpulseVector * OtherInverseMass;
	}

	void AddVelocity(const FVector& DeltaVelocity) override
	{
		Velocity += DeltaVelocity;
	}
	// 공기 저항과 마찰을 고려하지 않으므로 속도 감쇠 X & 각속도 고려 X
	void Move(float DeltaTime)
	{
		Location += Velocity * DeltaTime;
	}

	void CheckBorderCollision(float Left, float Right, float Top, float Bottom)
	{
		if (Location.x - Radius < Left)
		{
			Location.x = Left + Radius;
			if (Velocity.x < 0.0f)
			{
				Velocity.x *= -1.0f;
			}
		}
		else if (Location.x + Radius > Right)
		{
			Location.x = Right - Radius;
			if (Velocity.x > 0.0f)
			{
				Velocity.x *= -1.0f;
			}
		}

		if (Location.y - Radius < Top)
		{
			Location.y = Top + Radius;
			if (Velocity.y < 0.0f)
			{
				Velocity.y *= -1.0f;
			}
		}
		else if (Location.y + Radius > Bottom)
		{
			Location.y = Bottom - Radius;
			if (Velocity.y > 0.0f)
			{
				Velocity.y *= -1.0f;
			}
		}
	}

	void SetRadius(float NewRadius)
	{
		Radius = NewRadius > 0.0f ? NewRadius : 0.01f;
		Mass = Radius * Radius;
	}
};

int UBall::TotalNumBalls = 0;

float RandomFloat(float Min, float Max)
{
	return Min + ((float)rand() / (float)RAND_MAX) * (Max - Min);
}

UBall* CreateRandomBall()
{
	const float	  Radius = RandomFloat(0.05f, 0.15f);
	const FVector Location(
		RandomFloat(-1.0f + Radius, 1.0f - Radius),
		RandomFloat(-1.0f + Radius, 1.0f - Radius),
		0.0f);
	FVector Velocity(
		RandomFloat(-1.0f, 1.0f),
		RandomFloat(-1.0f, 1.0f),
		0.0f);

	return new UBall(Location, Velocity, Radius);
}

void ResizePrimitiveList(UPrimitive**& PrimitiveList, int TargetNumBalls)
{
	if (TargetNumBalls < 1)
	{
		TargetNumBalls = 1;
	}

	int CurrentNumBalls = UBall::TotalNumBalls;
	if (TargetNumBalls == CurrentNumBalls)
	{
		return;
	}

	if (TargetNumBalls < CurrentNumBalls)
	{
		while (CurrentNumBalls > TargetNumBalls)
		{
			const int RemoveIndex = rand() % CurrentNumBalls;
			delete PrimitiveList[RemoveIndex];
			PrimitiveList[RemoveIndex] = PrimitiveList[CurrentNumBalls - 1];
			--CurrentNumBalls;
		}
	}

	UPrimitive** TempPrimitiveList = new UPrimitive*[TargetNumBalls];
	const int	 NumBallsToCopy = CurrentNumBalls < TargetNumBalls ? CurrentNumBalls : TargetNumBalls;

	for (int i = 0; i < NumBallsToCopy; ++i)
	{
		TempPrimitiveList[i] = PrimitiveList[i];
	}

	for (int i = CurrentNumBalls; i < TargetNumBalls; ++i)
	{
		TempPrimitiveList[i] = CreateRandomBall();
	}

	delete[] PrimitiveList;
	PrimitiveList = TempPrimitiveList;
}

void ReleasePrimitiveList(UPrimitive**& PrimitiveList)
{
	while (UBall::TotalNumBalls > 0)
	{
		delete PrimitiveList[UBall::TotalNumBalls - 1];
	}

	delete[] PrimitiveList;
	PrimitiveList = nullptr;
}

// 마우스로 지나간 자리에 그려지는 공은 물리 공과 구분하여 위치만 관리
struct FBallBrush
{
	FVector* DrawingBallLocations = nullptr;
	int		 NumDrawingBalls = 0;
	int		 DrawingBallCapacity = 0;
	FVector	 LastDrawLocation;
	bool	 bHasLastDrawLocation = false;

	FBallBrush() = default;

	~FBallBrush()
	{
		delete[] DrawingBallLocations;
	}

	FBallBrush(const FBallBrush&) = delete;
	FBallBrush& operator=(const FBallBrush&) = delete;

	void AddDrawingBall(const FVector& Location)
	{
		if (NumDrawingBalls == DrawingBallCapacity)
		{
			const int NewCapacity = DrawingBallCapacity == 0 ? 64 : DrawingBallCapacity * 2;
			FVector*  TempDrawingBallLocations = new FVector[NewCapacity];

			for (int i = 0; i < NumDrawingBalls; ++i)
			{
				TempDrawingBallLocations[i] = DrawingBallLocations[i];
			}

			delete[] DrawingBallLocations;
			DrawingBallLocations = TempDrawingBallLocations;
			DrawingBallCapacity = NewCapacity;
		}

		DrawingBallLocations[NumDrawingBalls] = Location;
		++NumDrawingBalls;
	}

	void DrawTo(const FVector& Location, float Spacing)
	{
		if (!bHasLastDrawLocation)
		{
			AddDrawingBall(Location);
			LastDrawLocation = Location;
			bHasLastDrawLocation = true;
			return;
		}

		FVector Delta = Location - LastDrawLocation;
		float	Distance = sqrtf(FVector::DotProduct(Delta, Delta));

		// 마우스가 한 프레임에 멀리 이동해도 중간을 일정한 간격의 공으로 채움
		while (Distance >= Spacing)
		{
			const float MoveRatio = Spacing / Distance;
			LastDrawLocation += Delta * MoveRatio;
			AddDrawingBall(LastDrawLocation);

			Delta = Location - LastDrawLocation;
			Distance = sqrtf(FVector::DotProduct(Delta, Delta));
		}
	}

	void EndStroke()
	{
		bHasLastDrawLocation = false;
	}

	void Clear()
	{
		NumDrawingBalls = 0;
		EndStroke();
	}
};

bool ConvertMouseToWorldLocation(
	const ImVec2&		  MousePosition,
	const D3D11_VIEWPORT& Viewport,
	FVector&			  MouseWorldLocation)
{
	const bool bIsMouseInViewport =
		MousePosition.x >= Viewport.TopLeftX && MousePosition.x <= Viewport.TopLeftX + Viewport.Width && MousePosition.y >= Viewport.TopLeftY && MousePosition.y <= Viewport.TopLeftY + Viewport.Height;

	if (!bIsMouseInViewport || Viewport.Width <= 0.0f || Viewport.Height <= 0.0f)
	{
		return false;
	}

	MouseWorldLocation.x = ((MousePosition.x - Viewport.TopLeftX) / Viewport.Width) * 2.0f - 1.0f;
	MouseWorldLocation.y = 1.0f - ((MousePosition.y - Viewport.TopLeftY) / Viewport.Height) * 2.0f;
	MouseWorldLocation.z = 0.0f;
	return true;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 각종 메시지를 처리할 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}

	switch (message)
	{
		case WM_DESTROY:
			// Signal that the app should quit
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// 윈도우 클래스 이름
	WCHAR WindowClass[] = L"JungleWindowClass";

	// 윈도우 타이틀바에 표시될 이름
	WCHAR Title[] = L"Game Tech Lab";

	// 각종 메시지를 처리할 함수인 WndProc의 함수 포인터를 WindowClass 구조체에 넣는다.
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };

	// 윈도우 클래스 등록
	RegisterClassW(&wndclass);

	// 1024 x 1024 크기에 윈도우 생성
	HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024,
		nullptr, nullptr, hInstance, nullptr);

	// Renderer Class를 생성합니다.
	URenderer renderer;

	// D3D11 생성하는 함수를 호출합니다.
	renderer.Create(hWnd);

	// 렌더러 생성 직후에 쉐이더를 생성하는 함수를 호출합니다.
	renderer.CreateShader();

	renderer.CreateConstantBuffer();

	// 여기에서 ImGui를 생성합니다.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(renderer.Device, renderer.DeviceContext);

	UINT numVerticesSphere = sizeof(sphere_vertices) / sizeof(FVertexSimple);

	// 버텍스 버퍼로 넘기기 전에 Scale Down합니다.
	float scaleMod = 0.1f;

	for (UINT i = 0; i < numVerticesSphere; ++i)
	{
		sphere_vertices[i].x *= scaleMod;
		sphere_vertices[i].y *= scaleMod;
		sphere_vertices[i].z *= scaleMod;
	}

	// 버텍스 버퍼 하나만 생성
	ID3D11Buffer* vertexBufferSphere = renderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));

	// 화면의 경계 위치를 나타내는 상수 변수 설정(NDC 좌표계)
	const float leftBorder = -1.0f;
	const float rightBorder = 1.0f;
	const float topBorder = -1.0f;
	const float bottomBorder = 1.0f;

	bool		  bPinballMovement = true;
	bool		  bEnableCheckCollision = true;
	bool		  bEnableGravity = false;
	bool		  bEnableBallDrawing = false;
	bool		  bEnableBlackHole = false;
	const float	  BlackHoleStrength = 5.0f;
	const FVector GravityAcceleration(0.0f, -9.81f, 0.0f);
	const float	  DrawingBallRadius = 0.025f;
	const float	  DrawingBallSpacing = DrawingBallRadius * 1.5f;
	const float	  BlackHoleSoftening = 0.05f;
	const float	  MaxBlackHoleAcceleration = 30.0f;
	FBallBrush	  BallBrush;

	// 공은 조건에 따라 UPrimitive 이중 포인터로 관리
	srand(static_cast<unsigned int>(GetTickCount()));
	UPrimitive** PrimitiveList = nullptr;
	ResizePrimitiveList(PrimitiveList, 1);
	int DesiredNumBalls = UBall::TotalNumBalls;

	// FPS 제한을 위한 설정
	const int	 targetFPS = 30;
	const double targetFrameTime = 1000.0 / targetFPS; // 한 프레임의 목표 시간 (밀리초 단위)

	// 고성능 타이머 초기화
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER startTime, endTime;
	double		  elapsedTime = 0.0;

	bool bIsExit = false;

	// Main Loop (Quit Message가 들어오기 전까지 아래 Loop를 무한히 실행하게 됨)
	while (bIsExit == false)
	{
		// 루프 시작 시간 기록
		QueryPerformanceCounter(&startTime);

		MSG msg;

		// 처리할 메시지가 더 이상 없을때 까지 수행
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// 키 입력 메시지를 번역
			TranslateMessage(&msg);

			// 메시지를 적절한 윈도우 프로시저에 전달, 메시지가 위에서 등록한 WndProc 으로 전달됨
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				bIsExit = true;
				break;
			}
			else if (msg.message == WM_KEYDOWN) // 키보드 눌렸을 때
			{
				// 키보드 입력을 받을 공 지정
				UBall* ControlledBall = static_cast<UBall*>(PrimitiveList[0]);

				// 눌린 키가 방향키라면 해당 방향에 맞춰서
				// 첫 번째 공의 위치를 조정합니다.
				if (msg.wParam == VK_LEFT)
				{
					ControlledBall->Location.x -= 0.01f;
				}
				if (msg.wParam == VK_RIGHT)
				{
					ControlledBall->Location.x += 0.01f;
				}
				if (msg.wParam == VK_UP)
				{
					ControlledBall->Location.y += 0.01f;
				}
				if (msg.wParam == VK_DOWN)
				{
					ControlledBall->Location.y -= 0.01f;
				}
				// 키보드 처리 직후에 하면 밖을 벗어났다면 화면 안쪽으로 위치시킨다.
				// 화면을 벗어나지 않아야 한다면
				ControlledBall->CheckBorderCollision(leftBorder, rightBorder, topBorder, bottomBorder);
			}
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGuiIO&	 FrameIO = ImGui::GetIO();
		const ImVec2 MousePosition = FrameIO.MousePos;
		FVector		 MouseWorldLocation;
		const bool	 bHasMouseWorldLocation =
			ConvertMouseToWorldLocation(MousePosition, renderer.ViewportInfo, MouseWorldLocation);
		const bool bCanUseSceneMouse = bHasMouseWorldLocation && !FrameIO.WantCaptureMouse;
		const bool bIsBlackHoleActive =
			bEnableBlackHole && bCanUseSceneMouse && ImGui::IsMouseDown(ImGuiMouseButton_Right);

		if (bEnableBallDrawing && bCanUseSceneMouse && ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			BallBrush.DrawTo(MouseWorldLocation, DrawingBallSpacing);
		}
		else
		{
			BallBrush.EndStroke();
		}

		// 핀볼 움직임 || 중력 || 블랙홀이 켜져 있다면 물리 시뮬레이션을 갱신
		if (bPinballMovement || bEnableGravity || bIsBlackHoleActive)
		{
			const float	  DeltaTime = 1.0f / (float)targetFPS;
			const FVector GravityVelocityChange = GravityAcceleration * DeltaTime;

			for (int i = 0; i < UBall::TotalNumBalls; ++i)
			{
				UBall* Ball = static_cast<UBall*>(PrimitiveList[i]);
				if (bEnableGravity)
				{
					Ball->AddVelocity(GravityVelocityChange);
				}

				if (bIsBlackHoleActive)
				{
					const FVector Direction = MouseWorldLocation - Ball->Location;
					const float	  DistanceSquared = FVector::DotProduct(Direction, Direction);

					if (DistanceSquared > 0.000001f)
					{
						const FVector Normal = Direction / sqrtf(DistanceSquared);
						const float	  ForceFalloff = 1.0f / (DistanceSquared + BlackHoleSoftening);
						FVector		  BlackHoleAcceleration = Normal * (BlackHoleStrength * ForceFalloff);

						const float AccelerationSquared =
							FVector::DotProduct(BlackHoleAcceleration, BlackHoleAcceleration);
						if (AccelerationSquared > MaxBlackHoleAcceleration * MaxBlackHoleAcceleration)
						{
							BlackHoleAcceleration *= MaxBlackHoleAcceleration / sqrtf(AccelerationSquared);
						}

						Ball->AddVelocity(BlackHoleAcceleration * DeltaTime);
					}
				}

				Ball->Move(DeltaTime);
			}

			if (bEnableCheckCollision)
			{
				// 같은 공 쌍을 중복 처리하지 않도록 j는 i + 1부터 검사
				for (int i = 0; i < UBall::TotalNumBalls; ++i)
				{
					for (int j = i + 1; j < UBall::TotalNumBalls; ++j)
					{
						if (PrimitiveList[i]->IsColliding(PrimitiveList[j]))
						{
							PrimitiveList[i]->ResolveCollision(PrimitiveList[j]);
						}
					}
				}
			}

			// 공끼리의 위치 보정으로 벽 밖에 밀릴 수 있으므로 벽 충돌을 마지막에 처리
			for (int i = 0; i < UBall::TotalNumBalls; ++i)
			{
				UBall* Ball = static_cast<UBall*>(PrimitiveList[i]);
				Ball->CheckBorderCollision(leftBorder, rightBorder, topBorder, bottomBorder);
			}
		}

		////////////////////////////////////////////
		// 매번 실행되는 코드를 여기에 추가합니다.

		// 준비 작업
		renderer.Prepare();
		renderer.PrepareShader();

		// 하나의 버텍스 버퍼를 모든 공이 공유합니다.
		for (int i = 0; i < UBall::TotalNumBalls; ++i)
		{
			UBall* Ball = static_cast<UBall*>(PrimitiveList[i]);
			renderer.UpdateConstant(Ball->Location, Ball->Radius / scaleMod);
			renderer.RenderPrimitive(vertexBufferSphere, numVerticesSphere);
		}

		// 그림용 공도 기존 버텍스 버퍼를 공유하여 렌더링
		for (int i = 0; i < BallBrush.NumDrawingBalls; ++i)
		{
			renderer.UpdateConstant(BallBrush.DrawingBallLocations[i], DrawingBallRadius / scaleMod);
			renderer.RenderPrimitive(vertexBufferSphere, numVerticesSphere);
		}

		// 이후 ImGui UI 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render() 사이인 여기에 위치합니다.
		ImGui::Begin("Jungle Property Window");

		ImGui::Text("Hello Jungle World!");
		ImGui::Text("Total Balls: %d", UBall::TotalNumBalls);

		ImGui::SetNextItemWidth(120.0f);
		bool bBallCountChanged = ImGui::InputInt(
			"##NumberOfBalls",
			&DesiredNumBalls,
			0,
			0,
			ImGuiInputTextFlags_ParseEmptyRefVal);
		ImGui::SameLine();
		if (ImGui::Button("-##NumberOfBalls"))
		{
			--DesiredNumBalls;
			bBallCountChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("+##NumberOfBalls"))
		{
			++DesiredNumBalls;
			bBallCountChanged = true;
		}
		ImGui::SameLine();
		ImGui::Text("Number of Balls");

		if (bBallCountChanged)
		{
			if (DesiredNumBalls < 1)
			{
				DesiredNumBalls = 1;
			}

			ResizePrimitiveList(PrimitiveList, DesiredNumBalls);
			DesiredNumBalls = UBall::TotalNumBalls;
		}

		ImGui::Checkbox("Pinball Movement", &bPinballMovement);

		ImGui::Checkbox("Enable Collision", &bEnableCheckCollision);

		ImGui::Checkbox("Enable Gravity", &bEnableGravity);

		ImGui::Checkbox("Enable Ball Drawing", &bEnableBallDrawing);
		ImGui::SameLine();
		if (ImGui::Button("Clear Drawing"))
		{
			BallBrush.Clear();
		}
		ImGui::Text("Drawing Balls: %d", BallBrush.NumDrawingBalls);

		ImGui::Checkbox("Enable Black Hole", &bEnableBlackHole);

		ImGui::End();

		// 이후 ImGui UI 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render() 사이인 여기에 위치합니다.

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// 다 그렸으면 버퍼를 교환
		renderer.SwapBuffer();

		do
		{
			Sleep(0);

			// 루프 종료 시간 기록
			QueryPerformanceCounter(&endTime);

			// 한 프레임이 소요된 시간 계산 (밀리초 단위로 변환)
			elapsedTime = (endTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;
		}
		while (elapsedTime < targetFrameTime);

		////////////////////////////////////////////
	}

	// 여기에서 ImGui 소멸
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// 소멸하는 코드를 여기에 추가합니다.
	ReleasePrimitiveList(PrimitiveList);
	renderer.ReleaseVertexBuffer(vertexBufferSphere);
	renderer.ReleaseConstantBuffer();
	renderer.ReleaseShader();
	renderer.Release();

	return 0;
}
