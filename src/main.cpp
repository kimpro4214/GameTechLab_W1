#include <windows.h>

// D3D 사용에 필요한 라이브러리들을 링크합니다.
#pragma comment(lib, "d3d11")
#pragma comment(lib, "user32")

// D3D 사용에 필요한 헤더파일들을 포함합니다.
#include <d3d11.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "FVector.h"
#include "FVertexSimple.h"
#include "Sphere.h"
#include "URenderer.h"

class UPrimitive
{
public:
	virtual ~UPrimitive() = default;
	virtual bool IsColliding(const UPrimitive* Other) const = 0;
	virtual void ResolveCollision(UPrimitive* Other, float Restitution, float FrictionCoefficient) = 0;
	virtual void AddVelocity(const FVector& DeltaVelocity) = 0;
};

class UBall : public UPrimitive
{
public:
	FVector				Location;
	FVector				Velocity;
	float				Radius;
	float				Mass;
	float				RotationAngle;
	float				AngularVelocity;
	FVector				LastCollisionPoint;
	FVector				LastCollisionNormal;
	FVector				LastCollisionTangent;
	bool				bHasCollisionDebug;
	static const float	DropTime;
	static bool			bCanDropBall;
	static int			NextLevel;
	static int			TotalNumBalls;
	static int			TotalScore;
	static int			CurrentIndex;
	static int			StorageLevel;
	static const float	BallSizes[11];
	static const int	ScoreList[11];
	int					Level;


	UBall(const FVector& InitialLocation = FVector(), const FVector& InitialVelocity = FVector(), int InitialLevel = 0)
		: Location(InitialLocation), Velocity(InitialVelocity), Level(InitialLevel), Radius(0.0f), Mass(0.0f),
		RotationAngle(0.0f), AngularVelocity(0.0f), bHasCollisionDebug(false)
	{
		SetRadius(BallSizes[Level]);
		CurrentIndex = TotalNumBalls;
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

	bool IsMergeable(const UBall* OtherBall)
	{
		return (this->Level < 11 && this->Radius == OtherBall->Radius);
	}

	void Merge(UPrimitive**& PrimitiveList, int OtherBall)
	{
		UBall::TotalScore += UBall::ScoreList[this->Level];
		this->Level++;
		SetRadius(UBall::BallSizes[Level]);

		delete PrimitiveList[OtherBall];
		if (OtherBall != UBall::TotalNumBalls)
		{
			PrimitiveList[OtherBall] = PrimitiveList[CurrentIndex];
			PrimitiveList[CurrentIndex] = PrimitiveList[UBall::TotalNumBalls];
			CurrentIndex = OtherBall;
		}
		PrimitiveList[UBall::TotalNumBalls] = nullptr;
	}

	void ResolveCollision(UPrimitive* Other, float Restitution, float FrictionCoefficient) override
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

		const FVector RelativeVelocity = OtherBall->Velocity - Velocity;

		float	Distance = 0.0f;
		FVector CollisionNormal(1.0f, 0.0f, 0.0f);

		const float Epsilon = 0.000001f;
		if (DistanceSquared > Epsilon)
		{
			Distance = sqrtf(DistanceSquared);
			CollisionNormal = Delta / Distance;
		}
		else
		{
			const float RelativeSpeedSquared = FVector::DotProduct(RelativeVelocity, RelativeVelocity);
			if (RelativeSpeedSquared > Epsilon)
			{
				CollisionNormal = RelativeVelocity * (-1.0f / sqrtf(RelativeSpeedSquared));
			}
		}

		const float InverseMass = 1.0f / Mass;
		const float OtherInverseMass = 1.0f / OtherBall->Mass;
		const float InverseMomentOfInertia = 1.0f / GetMomentOfInertia();
		const float OtherInverseMomentOfInertia = 1.0f / OtherBall->GetMomentOfInertia();
		const float InverseMassSum = InverseMass + OtherInverseMass;

		// 겹친 거리를 질량 비율에 따라 나누어 두 공을 서로 밀어냄
		const float Penetration = RadiusSum - Distance;
		const float PenetrationSlop = 0.001f;
		const float CorrectionPercent = 0.25f;
		const float CorrectedPenetration =
			Penetration > PenetrationSlop ? Penetration - PenetrationSlop : 0.0f;
		const FVector Correction =
			CollisionNormal * (CorrectedPenetration * CorrectionPercent / InverseMassSum);
		Location -= Correction * InverseMass;
		OtherBall->Location += Correction * OtherInverseMass;

		const FVector ContactOffset = CollisionNormal * Radius;
		const FVector OtherContactOffset = CollisionNormal * -OtherBall->Radius;
		const FVector ContactVelocity =
			Velocity + FVector::CrossProduct2D(AngularVelocity, ContactOffset);
		const FVector OtherContactVelocity =
			OtherBall->Velocity +
			FVector::CrossProduct2D(OtherBall->AngularVelocity, OtherContactOffset);
		const FVector RelativeContactVelocity = OtherContactVelocity - ContactVelocity;
		const float VelocityAlongNormal =
			FVector::DotProduct(RelativeContactVelocity, CollisionNormal);

		LastCollisionPoint = Location + ContactOffset;
		LastCollisionNormal = CollisionNormal;
		LastCollisionTangent = FVector(-CollisionNormal.y, CollisionNormal.x, 0.0f);
		bHasCollisionDebug = true;
		OtherBall->LastCollisionPoint = LastCollisionPoint;
		OtherBall->LastCollisionNormal = CollisionNormal * -1.0f;
		OtherBall->LastCollisionTangent = LastCollisionTangent;
		OtherBall->bHasCollisionDebug = true;

		// 이미 서로 멀어지는 중이라면 위치만 보정하고 추가 충격량은 적용 X
		if (VelocityAlongNormal >= 0.0f)
		{
			return;
		}

		// 공기 저항 없고, 마찰 없고, 공 끼리 탄성 충돌을 하기에 반발계수를 1로 설정
		// 반발계수 1인 완전 탄성 충돌의 충격량을 계산
		const float ContactNormalCross = FVector::CrossProduct2D(ContactOffset, CollisionNormal);
		const float OtherContactNormalCross = FVector::CrossProduct2D(OtherContactOffset, CollisionNormal);
		const float NormalImpulseDenominator =
			InverseMassSum +
			ContactNormalCross * ContactNormalCross * InverseMomentOfInertia +
			OtherContactNormalCross * OtherContactNormalCross * OtherInverseMomentOfInertia;
		if (NormalImpulseDenominator <= Epsilon)
		{
			return;
		}

		const float NormalImpulseMagnitude =
			-(1.0f + Restitution) * VelocityAlongNormal / NormalImpulseDenominator;
		const FVector NormalImpulse = CollisionNormal * NormalImpulseMagnitude;
		Velocity -= NormalImpulse * InverseMass;
		OtherBall->Velocity += NormalImpulse * OtherInverseMass;
		AngularVelocity -=
			FVector::CrossProduct2D(ContactOffset, NormalImpulse) * InverseMomentOfInertia;
		OtherBall->AngularVelocity +=
			FVector::CrossProduct2D(OtherContactOffset, NormalImpulse) * OtherInverseMomentOfInertia;

		const FVector NewContactVelocity =
			Velocity + FVector::CrossProduct2D(AngularVelocity, ContactOffset);
		const FVector NewOtherContactVelocity =
			OtherBall->Velocity +
			FVector::CrossProduct2D(OtherBall->AngularVelocity, OtherContactOffset);
		const FVector NewRelativeContactVelocity = NewOtherContactVelocity - NewContactVelocity;
		const float NewVelocityAlongNormal =
			FVector::DotProduct(NewRelativeContactVelocity, CollisionNormal);
		const FVector TangentVelocity =
			NewRelativeContactVelocity - CollisionNormal * NewVelocityAlongNormal;
		const float TangentSpeedSquared = FVector::DotProduct(TangentVelocity, TangentVelocity);

		if (TangentSpeedSquared > Epsilon)
		{
			const FVector Tangent = TangentVelocity / sqrtf(TangentSpeedSquared);
			LastCollisionTangent = Tangent;
			OtherBall->LastCollisionTangent = Tangent;

			const float ContactTangentCross = FVector::CrossProduct2D(ContactOffset, Tangent);
			const float OtherContactTangentCross = FVector::CrossProduct2D(OtherContactOffset, Tangent);
			const float TangentialImpulseDenominator =
				InverseMassSum +
				ContactTangentCross * ContactTangentCross * InverseMomentOfInertia +
				OtherContactTangentCross * OtherContactTangentCross * OtherInverseMomentOfInertia;
			if (TangentialImpulseDenominator <= Epsilon)
			{
				return;
			}

			float TangentialImpulseMagnitude =
				-FVector::DotProduct(NewRelativeContactVelocity, Tangent) /
				TangentialImpulseDenominator;
			const float MaxFrictionImpulse = FrictionCoefficient * NormalImpulseMagnitude;
			if (TangentialImpulseMagnitude > MaxFrictionImpulse)
			{
				TangentialImpulseMagnitude = MaxFrictionImpulse;
			}
			else if (TangentialImpulseMagnitude < -MaxFrictionImpulse)
			{
				TangentialImpulseMagnitude = -MaxFrictionImpulse;
			}

			const FVector FrictionImpulse = Tangent * TangentialImpulseMagnitude;
			Velocity -= FrictionImpulse * InverseMass;
			OtherBall->Velocity += FrictionImpulse * OtherInverseMass;
			AngularVelocity -=
				FVector::CrossProduct2D(ContactOffset, FrictionImpulse) * InverseMomentOfInertia;
			OtherBall->AngularVelocity +=
				FVector::CrossProduct2D(OtherContactOffset, FrictionImpulse) * OtherInverseMomentOfInertia;
		}
	}

	void AddVelocity(const FVector& DeltaVelocity) override
	{
		Velocity += DeltaVelocity;
	}

	float GetMomentOfInertia() const
	{
		return 0.5f * Mass * Radius * Radius;
	}

	void AddTorque(float Torque, float DeltaTime)
	{
		const float AngularAcceleration = Torque / GetMomentOfInertia();
		AngularVelocity += AngularAcceleration * DeltaTime;
	}

	// 공기 저항과 마찰을 고려하지 않으므로 속도 감쇠 X & 각속도 고려 X
	void Move(float DeltaTime, float AngularDamping)
	{
		Location += Velocity * DeltaTime;
		AngularVelocity /= 1.0f + AngularDamping * DeltaTime;
		RotationAngle += AngularVelocity * DeltaTime;
		RotationAngle = fmodf(RotationAngle, 6.28318530718f);
	}

	void ResolveBorderContact(const FVector& CollisionNormal, float Restitution,
		float FrictionCoefficient)
	{
		const float Epsilon = 0.000001f;
		const float InverseMass = 1.0f / Mass;
		const float InverseMomentOfInertia = 1.0f / GetMomentOfInertia();
		const FVector ContactOffset = CollisionNormal * -Radius;
		FVector ContactVelocity =
			Velocity + FVector::CrossProduct2D(AngularVelocity, ContactOffset);
		const float VelocityAlongNormal = FVector::DotProduct(ContactVelocity, CollisionNormal);
		if (VelocityAlongNormal >= 0.0f)
		{
			return;
		}

		const float ContactNormalCross = FVector::CrossProduct2D(ContactOffset, CollisionNormal);
		const float NormalImpulseDenominator =
			InverseMass + ContactNormalCross * ContactNormalCross * InverseMomentOfInertia;
		if (NormalImpulseDenominator <= Epsilon)
		{
			return;
		}

		const float NormalImpulseMagnitude =
			-(1.0f + Restitution) * VelocityAlongNormal / NormalImpulseDenominator;
		const FVector NormalImpulse = CollisionNormal * NormalImpulseMagnitude;
		Velocity += NormalImpulse * InverseMass;
		AngularVelocity +=
			FVector::CrossProduct2D(ContactOffset, NormalImpulse) * InverseMomentOfInertia;

		ContactVelocity = Velocity + FVector::CrossProduct2D(AngularVelocity, ContactOffset);
		const float NewVelocityAlongNormal = FVector::DotProduct(ContactVelocity, CollisionNormal);
		const FVector TangentVelocity = ContactVelocity - CollisionNormal * NewVelocityAlongNormal;
		const float TangentSpeedSquared = FVector::DotProduct(TangentVelocity, TangentVelocity);
		if (TangentSpeedSquared <= Epsilon)
		{
			return;
		}

		const FVector Tangent = TangentVelocity / sqrtf(TangentSpeedSquared);
		const float ContactTangentCross = FVector::CrossProduct2D(ContactOffset, Tangent);
		const float TangentialImpulseDenominator =
			InverseMass + ContactTangentCross * ContactTangentCross * InverseMomentOfInertia;
		if (TangentialImpulseDenominator <= Epsilon)
		{
			return;
		}

		float TangentialImpulseMagnitude =
			-FVector::DotProduct(ContactVelocity, Tangent) / TangentialImpulseDenominator;
		const float MaxFrictionImpulse = FrictionCoefficient * NormalImpulseMagnitude;
		if (TangentialImpulseMagnitude > MaxFrictionImpulse)
		{
			TangentialImpulseMagnitude = MaxFrictionImpulse;
		}
		else if (TangentialImpulseMagnitude < -MaxFrictionImpulse)
		{
			TangentialImpulseMagnitude = -MaxFrictionImpulse;
		}

		const FVector FrictionImpulse = Tangent * TangentialImpulseMagnitude;
		Velocity += FrictionImpulse * InverseMass;
		AngularVelocity +=
			FVector::CrossProduct2D(ContactOffset, FrictionImpulse) * InverseMomentOfInertia;
	}

	void CheckBorderCollision(float Left, float Right, float Top, float Bottom,
		float Restitution, float FrictionCoefficient)
	{
		if (Location.x - Radius < Left)
		{
			Location.x = Left + Radius;
			ResolveBorderContact(FVector(1.0f, 0.0f, 0.0f), Restitution, FrictionCoefficient);
		}
		else if (Location.x + Radius > Right)
		{
			Location.x = Right - Radius;
			ResolveBorderContact(FVector(-1.0f, 0.0f, 0.0f), Restitution, FrictionCoefficient);
		}

		if (Location.y - Radius < Top)
		{
			Location.y = Top + Radius;
			ResolveBorderContact(FVector(0.0f, 1.0f, 0.0f), Restitution, FrictionCoefficient);
		}
		else if (Location.y + Radius > Bottom)
		{
			Location.y = Bottom - Radius;
			ResolveBorderContact(FVector(0.0f, -1.0f, 0.0f), Restitution, FrictionCoefficient);
		}
	}

	void SetRadius(float NewRadius)
	{
		Radius = NewRadius > 0.0f ? NewRadius : 0.01f;
		Mass = Radius * Radius;
	}

	static void Swap(UPrimitive**& PrimitiveList)
	{
		UBall* CurrentBall = dynamic_cast<UBall*>(PrimitiveList[UBall::CurrentIndex]);
		if (CurrentBall == nullptr)
		{
			return ;
		}
		if (UBall::StorageLevel == -1)
		{
			UBall::StorageLevel = CurrentBall->Level;
			CurrentBall->Level = UBall::NextLevel;
			UBall::NextLevel = rand() % 5;
		}
		else
		{
			int temp = CurrentBall->Level;
			CurrentBall->Level = UBall::StorageLevel;
			UBall::StorageLevel = temp;
		}
		const FVector NewLocation(
			0.0f - (UBall::BallSizes[CurrentBall->Level] * 0.5f),
			0.9f,
			0.0f);
		CurrentBall->Location = NewLocation;
		CurrentBall->SetRadius(UBall::BallSizes[CurrentBall->Level]);
		return ;
	}
};

FVector GetFruitColor(float Radius)
{
	constexpr float ScaleStandard = 16.5f / 0.05f;
	const float StandardizedScale = Radius * ScaleStandard;

	if (StandardizedScale <= 16.5f) return FVector(0.95f, 0.05f, 0.05f);
	if (StandardizedScale <= 24.0f) return FVector(0.99f, 0.41f, 0.30f);
	if (StandardizedScale <= 30.5f) return FVector(0.63f, 0.42f, 1.00f);
	if (StandardizedScale <= 36.5f) return FVector(1.00f, 0.72f, 0.00f);
	if (StandardizedScale <= 44.5f) return FVector(0.99f, 0.55f, 0.17f);
	if (StandardizedScale <= 57.0f) return FVector(0.85f, 0.35f, 0.75f);
	if (StandardizedScale <= 64.5f) return FVector(0.98f, 0.94f, 0.62f);
	if (StandardizedScale <= 78.0f) return FVector(1.00f, 0.71f, 0.68f);
	if (StandardizedScale <= 88.5f) return FVector(0.97f, 0.92f, 0.04f);
	if (StandardizedScale <= 110.0f) return FVector(0.62f, 0.87f, 0.07f);

	return FVector(0.08f, 0.61f, 0.04f);
}

constexpr float FruitPreviewSize = 40.0f;
constexpr float FruitPreviewRadius = 18.0f;

void DrawFruitPreview(const FVector& FruitColor)
{
	const ImVec2 PreviewSize(FruitPreviewSize, FruitPreviewSize);
	const ImVec2 PreviewPosition = ImGui::GetCursorScreenPos();
	const ImVec2 PreviewCenter(
		PreviewPosition.x + PreviewSize.x * 0.5f,
		PreviewPosition.y + PreviewSize.y * 0.5f);
	ImDrawList* PreviewDrawList = ImGui::GetWindowDrawList();

	for (int PixelY = -18; PixelY <= 18; ++PixelY)
	{
		const float VerticalOffset = static_cast<float>(PixelY);
		const float HalfWidth = sqrtf(
			FruitPreviewRadius * FruitPreviewRadius - VerticalOffset * VerticalOffset);
		const float GradientT = 0.7f - VerticalOffset / (2.0f * FruitPreviewRadius);
		const ImVec4 GradientColor(
			1.0f + (FruitColor.x - 1.0f) * GradientT,
			1.0f + (FruitColor.y - 1.0f) * GradientT,
			1.0f + (FruitColor.z - 1.0f) * GradientT,
			1.0f);

		PreviewDrawList->AddLine(
			ImVec2(PreviewCenter.x - HalfWidth, PreviewCenter.y + VerticalOffset),
			ImVec2(PreviewCenter.x + HalfWidth, PreviewCenter.y + VerticalOffset),
			ImGui::ColorConvertFloat4ToU32(GradientColor), 1.0f);
	}

	PreviewDrawList->AddCircle(
		PreviewCenter, FruitPreviewRadius, IM_COL32(255, 255, 255, 255));
	ImGui::Dummy(PreviewSize);
}

bool		UBall::bCanDropBall = true;
int			UBall::TotalNumBalls = 0;
int			UBall::TotalScore = 0;
int			UBall::CurrentIndex = 0;
int			UBall::NextLevel = rand() % 5;
int			UBall::StorageLevel = -1;
const float	UBall::BallSizes[11] = { 0.05f, 0.07f, 0.09f, 0.11f, 0.13f, 0.16f, 0.19f, 0.22f, 0.25f, 0.3f, 0.4f };
const int	UBall::ScoreList[11] = { 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66 };
const float UBall::DropTime = 600.0f;

float RandomFloat(float Min, float Max) // 사용 안하는데 나중에 사용할 지 몰라서 삭제 보류
{
	return Min + ((float)rand() / (float)RAND_MAX) * (Max - Min);
}

UBall* CreateRandomBall()
{
	int CurrentLevel = UBall::NextLevel;
	UBall::NextLevel = rand() % 5;
	const FVector Location(
		0.0f - (UBall::BallSizes[CurrentLevel] * 0.5f),
		0.9f,
		0.0f);
	FVector Velocity(
		0.0f,
		0.0f,
		0.0f);

	return new UBall(Location, Velocity, CurrentLevel);
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

	UPrimitive** TempPrimitiveList = new UPrimitive * [TargetNumBalls];
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

bool ConvertMouseToWorldLocation(
	const ImVec2& MousePosition,
	const D3D11_VIEWPORT& Viewport,
	FVector& MouseWorldLocation)
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

ImVec2 ConvertWorldToScreenLocation(const FVector& WorldLocation, const D3D11_VIEWPORT& Viewport)
{
	return ImVec2(
		Viewport.TopLeftX + (WorldLocation.x + 1.0f) * 0.5f * Viewport.Width,
		Viewport.TopLeftY + (1.0f - WorldLocation.y) * 0.5f * Viewport.Height);
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

	renderer.InitImGui(hWnd);

	UINT numVerticesSphere = sizeof(sphere_vertices) / sizeof(FVertexSimple);

	// 버텍스 버퍼 하나만 생성
	ID3D11Buffer* vertexBufferSphere = renderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));

	// 화면의 경계 위치를 나타내는 상수 변수 설정(NDC 좌표계)
	const float leftBorder = -1.0f;
	const float rightBorder = 0.5f;
	const float topBorder = -1.0f;
	const float bottomBorder = 1.0f;

	const FVector GravityAcceleration(0.0f, -9.81f, 0.0f);
	bool bEnableTestTorque = false;
	float TestTorque = 0.001f;
	float Restitution = 1.0f;
	float FrictionCoefficient = 0.0f;
	float AngularDamping = 0.0f;
	bool bShowCollisionDebug = true;
	const int PhysicsSubsteps = 2;
	const int CollisionSolverIterations = 8;

	const float FruitRad[11] = { 0.05f, 0.07f, 0.075f, 0.1f, 0.125f, 0.15f, 0.175f, 0.2f, 0.25f, 0.275f, 0.4f };

	// 공은 조건에 따라 UPrimitive 이중 포인터로 관리
	srand(static_cast<unsigned int>(GetTickCount()));
	UPrimitive** PrimitiveList = nullptr;
	ResizePrimitiveList(PrimitiveList, 1);
	bool bIsDraggingBall = false;

	// FPS 제한을 위한 설정
	const int	 targetFPS = 30;
	const double targetFrameTime = 1000.0 / targetFPS; // 한 프레임의 목표 시간 (밀리초 단위)

	// 고성능 타이머 초기화
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER startTime, endTime, dropTime;
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
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGuiIO& FrameIO = ImGui::GetIO();
		const ImVec2 MousePosition = FrameIO.MousePos;
		FVector		 MouseWorldLocation;
		const bool	 bHasMouseWorldLocation =
			ConvertMouseToWorldLocation(MousePosition, renderer.ViewportInfo, MouseWorldLocation);
		const bool bIsMouseInGameBounds =
			bHasMouseWorldLocation &&
			MouseWorldLocation.x >= leftBorder && MouseWorldLocation.x <= rightBorder &&
			MouseWorldLocation.y >= topBorder && MouseWorldLocation.y <= bottomBorder;
		const bool bCanUseSceneMouse = bIsMouseInGameBounds && !FrameIO.WantCaptureMouse;
		int DesiredNumBalls = UBall::TotalNumBalls;

		if (bCanUseSceneMouse && ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			UBall* CurrentBall = static_cast<UBall*>(PrimitiveList[UBall::CurrentIndex]);
			CurrentBall->Location.x = MouseWorldLocation.x;
			bIsDraggingBall = true;
		}

		// 게임 영역에서 잡은 공은 경계 밖에서 마우스를 놓아도 떨어집니다.
		if (UBall::bCanDropBall && bIsDraggingBall && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			++DesiredNumBalls;
			ResizePrimitiveList(PrimitiveList, DesiredNumBalls);
			DesiredNumBalls = UBall::TotalNumBalls;
			bIsDraggingBall = false;
			UBall::bCanDropBall = false;
			QueryPerformanceCounter(&dropTime);
		}

		if (bCanUseSceneMouse && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		{
			char Buffer[256];
			sprintf_s(Buffer, "Swap 진행 \n");
			OutputDebugStringA(Buffer);
			UBall::Swap(PrimitiveList);
		}

		// 핀볼 움직임 || 중력 || 블랙홀이 켜져 있다면 물리 시뮬레이션을 갱신
		const float FrameDeltaTime = 1.0f / (float)targetFPS;
		const float SubstepDeltaTime = FrameDeltaTime / (float)PhysicsSubsteps;
		const FVector GravityVelocityChange = GravityAcceleration * SubstepDeltaTime;

		for (int i = 0; i < UBall::TotalNumBalls; ++i)
		{
			UBall* Ball = static_cast<UBall*>(PrimitiveList[i]);
			Ball->bHasCollisionDebug = false;
		}

		// substep 단위로 물리 시뮬레이션을 반복 수행
		for (int Substep = 0; Substep < PhysicsSubsteps; ++Substep)
		{
			for (int i = 0; i < UBall::TotalNumBalls; ++i)
			{
				if (i == UBall::CurrentIndex) // 대기중인 볼 제외
				{
					continue;
				}
				UBall* Ball = static_cast<UBall*>(PrimitiveList[i]);

				Ball->AddVelocity(GravityVelocityChange);
				if (bEnableTestTorque)
				{
					Ball->AddTorque(TestTorque, SubstepDeltaTime);
				}
				Ball->Move(SubstepDeltaTime, AngularDamping);
			}

			// 반복계수와 마찰계수를 고려하여 충돌을 해결하는 솔버 반복
			for (int SolverIteration = 0;
				SolverIteration < CollisionSolverIterations;
				++SolverIteration)
			{

				int TotalBall = UBall::TotalNumBalls;
				for (int i = 0; i < TotalBall; ++i)
				{
					if (i == UBall::CurrentIndex) // 대기중인 볼 제외
					{
						continue;
					}
					for (int j = i + 1; j < TotalBall; ++j)
					{
						if (j == UBall::CurrentIndex) // 대기중인 볼 제외
						{
							continue;
						}
						if (PrimitiveList[i]->IsColliding(PrimitiveList[j]))
						{

							UBall* BallA = static_cast<UBall*>(PrimitiveList[i]);
							UBall* BallB = static_cast<UBall*>(PrimitiveList[j]);
							if (BallA->IsMergeable(BallB))
							{
								BallA->Merge(PrimitiveList, j);
								TotalBall--;
								break;
							}
							PrimitiveList[i]->ResolveCollision(
								PrimitiveList[j], Restitution, FrictionCoefficient);
						}
					}
				}

				// 위치 보정으로 벽 밖에 밀린 공도 같은 솔버 반복 안에서 다시 해결합니다.
				for (int i = 0; i < UBall::TotalNumBalls; ++i)
				{
					UBall* Ball = static_cast<UBall*>(PrimitiveList[i]);
					Ball->CheckBorderCollision(
						leftBorder, rightBorder, topBorder, bottomBorder,
						Restitution, FrictionCoefficient);
				}
			}
		}
		// 준비 작업
		renderer.Prepare();
		renderer.PrepareShader();

		// 하나의 버텍스 버퍼를 모든 공이 공유합니다.
		for (int i = 0; i < UBall::TotalNumBalls; ++i)
		{
			UBall* Ball = static_cast<UBall*>(PrimitiveList[i]);
			renderer.UpdateConstant(Ball->Location, Ball->Radius, Ball->RotationAngle,
				GetFruitColor(Ball->Radius));
			renderer.RenderPrimitive(vertexBufferSphere, numVerticesSphere);
		}

		UBall* DebugBall = static_cast<UBall*>(PrimitiveList[0]);
		if (bShowCollisionDebug && DebugBall->bHasCollisionDebug)
		{
			const float DebugVectorLength = 0.15f;
			const ImVec2 ContactScreen = ConvertWorldToScreenLocation(
				DebugBall->LastCollisionPoint, renderer.ViewportInfo);
			const ImVec2 NormalScreen = ConvertWorldToScreenLocation(
				DebugBall->LastCollisionPoint +
				DebugBall->LastCollisionNormal * DebugVectorLength,
				renderer.ViewportInfo);
			const ImVec2 TangentScreen = ConvertWorldToScreenLocation(
				DebugBall->LastCollisionPoint +
				DebugBall->LastCollisionTangent * DebugVectorLength,
				renderer.ViewportInfo);
			ImDrawList* DebugDrawList = ImGui::GetForegroundDrawList();
			DebugDrawList->AddCircleFilled(ContactScreen, 4.0f, IM_COL32(255, 80, 80, 255));
			DebugDrawList->AddLine(ContactScreen, NormalScreen, IM_COL32(80, 255, 80, 255), 2.0f);
			DebugDrawList->AddLine(ContactScreen, TangentScreen, IM_COL32(255, 220, 80, 255), 2.0f);
		}

		const ImVec2 GameTopLeft = ConvertWorldToScreenLocation(
			FVector(leftBorder, topBorder, 0.0f), renderer.ViewportInfo);
		const ImVec2 GameBottomRight = ConvertWorldToScreenLocation(
			FVector(rightBorder, bottomBorder, 0.0f), renderer.ViewportInfo);
		ImGui::GetForegroundDrawList()->AddRect(
			GameTopLeft, GameBottomRight, IM_COL32(255, 255, 255, 180), 0.0f, 0, 2.0f);

		// 이후 ImGui UI 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render() 사이인 여기에 위치합니다.
		ImGui::SetNextWindowPos(
			ImVec2(renderer.ViewportInfo.Width * 0.75f, 0.0f), ImGuiCond_Always);
		ImGui::SetNextWindowSize(
			ImVec2(renderer.ViewportInfo.Width * 0.25f, renderer.ViewportInfo.Height),
			ImGuiCond_Always);
		ImGui::Begin("Jungle Property Window");

		ImGui::Text("Total Score : %d", UBall::TotalScore);

		const FVector NextFruitColor = GetFruitColor(UBall::BallSizes[UBall::NextLevel]);
		ImGui::Text("Next Fruit Color");
		DrawFruitPreview(NextFruitColor);

		ImGui::Text("Storage Fruit Color (RightClick)");
		if (UBall::StorageLevel == -1)
		{
			ImGui::Text("Empty");
		}
		else
		{
			const FVector StorageFruitColor = GetFruitColor(UBall::BallSizes[UBall::StorageLevel]);
			DrawFruitPreview(StorageFruitColor);
		}
		ImGui::Text("Angle: %.3f, Angular Velocity: %.3f",
			DebugBall->RotationAngle, DebugBall->AngularVelocity);
		ImGui::Checkbox("Enable Test Torque", &bEnableTestTorque);
		ImGui::DragFloat("Test Torque", &TestTorque, 0.0001f, -0.01f, 0.01f, "%.4f");
		ImGui::SliderFloat("Restitution", &Restitution, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Friction", &FrictionCoefficient, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Angular Damping", &AngularDamping, 0.0f, 5.0f, "%.2f");
		ImGui::Text("Physics Substeps: %d", PhysicsSubsteps);
		ImGui::Text("Collision Solver Iterations: %d", CollisionSolverIterations);
		ImGui::Checkbox("Show Collision Debug", &bShowCollisionDebug);
		if (ImGui::Button("Reset Rotation"))
		{
			DebugBall->RotationAngle = 0.0f;
			DebugBall->AngularVelocity = 0.0f;
		}
		ImGui::Text("Fruit Sequence");
		const int FruitCount = sizeof(FruitRad) / sizeof(FruitRad[0]);
		for (int i = 0; i < FruitCount; ++i)
		{
			const FVector FruitColor = GetFruitColor(FruitRad[i]);
			DrawFruitPreview(FruitColor);
			const bool bHasNextFruit = i < FruitCount - 1;
			const bool bIsEndOfRow = (i + 1) % 3 == 0;
			if (bHasNextFruit)
			{
				ImGui::SameLine();
				const float ArrowOffsetY =
					(FruitPreviewSize - ImGui::GetFrameHeight()) * 0.5f;
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ArrowOffsetY);

				ImGui::PushID(i);
				ImGui::ArrowButton("Right", ImGuiDir_Right);
				ImGui::PopID();
				if (!bIsEndOfRow)
				{
					ImGui::SameLine();
				}
			}
		}
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
			if (!UBall::bCanDropBall)
			{
				double DropElapsedTime = (endTime.QuadPart - dropTime.QuadPart) * 1000.0 / frequency.QuadPart;
				if (DropElapsedTime >= UBall::DropTime)
				{
					UBall::bCanDropBall = true;
				}
			}
		} while (elapsedTime < targetFrameTime);
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
