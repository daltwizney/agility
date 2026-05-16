#include "FlappyBird.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	void EmitSphere(
		const FVector& Center,
		const FVector& Radii,
		int32 Rings,
		int32 Segments,
		const FLinearColor& Color,
		TArray<FVector>& Vertices,
		TArray<int32>& Triangles,
		TArray<FVector>& Normals,
		TArray<FVector2D>& UV0,
		TArray<FLinearColor>& Colors)
	{
		Rings = FMath::Max(Rings, 3);
		Segments = FMath::Max(Segments, 4);

		const int32 BaseIndex = Vertices.Num();

		for (int32 r = 0; r <= Rings; ++r)
		{
			const float V = static_cast<float>(r) / Rings;
			const float Lat = -PI * 0.5f + V * PI;
			const float CosLat = FMath::Cos(Lat);
			const float SinLat = FMath::Sin(Lat);

			for (int32 s = 0; s <= Segments; ++s)
			{
				const float U = static_cast<float>(s) / Segments;
				const float Lon = U * 2.0f * PI;
				const float CosLon = FMath::Cos(Lon);
				const float SinLon = FMath::Sin(Lon);

				const FVector UnitDir(CosLat * CosLon, CosLat * SinLon, SinLat);
				const FVector Local(UnitDir.X * Radii.X, UnitDir.Y * Radii.Y, UnitDir.Z * Radii.Z);

				Vertices.Add(Center + Local);
				// Normal of a stretched sphere isn't just the radial direction; rebuild it from the implicit-surface gradient.
				const FVector Normal = FVector(UnitDir.X / Radii.X, UnitDir.Y / Radii.Y, UnitDir.Z / Radii.Z).GetSafeNormal();
				Normals.Add(Normal);
				UV0.Add(FVector2D(U, V));
				Colors.Add(Color);
			}
		}

		const int32 Stride = Segments + 1;
		for (int32 r = 0; r < Rings; ++r)
		{
			for (int32 s = 0; s < Segments; ++s)
			{
				const int32 I0 = BaseIndex + r * Stride + s;
				const int32 I1 = BaseIndex + (r + 1) * Stride + s;
				const int32 I2 = BaseIndex + r * Stride + (s + 1);
				const int32 I3 = BaseIndex + (r + 1) * Stride + (s + 1);

				Triangles.Add(I0);
				Triangles.Add(I1);
				Triangles.Add(I3);

				Triangles.Add(I0);
				Triangles.Add(I3);
				Triangles.Add(I2);
			}
		}
	}

	void EmitCone(
		const FVector& Apex,
		const FVector& BaseCenter,
		float BaseRadius,
		int32 Sides,
		const FLinearColor& Color,
		TArray<FVector>& Vertices,
		TArray<int32>& Triangles,
		TArray<FVector>& Normals,
		TArray<FVector2D>& UV0,
		TArray<FLinearColor>& Colors)
	{
		Sides = FMath::Max(Sides, 3);

		const FVector Axis = (Apex - BaseCenter).GetSafeNormal();
		if (Axis.IsNearlyZero())
		{
			return;
		}

		const FVector Helper = FMath::Abs(Axis.Z) < 0.99f ? FVector::UpVector : FVector::ForwardVector;
		const FVector Tangent = FVector::CrossProduct(Axis, Helper).GetSafeNormal();
		const FVector Bitangent = FVector::CrossProduct(Axis, Tangent).GetSafeNormal();

		const int32 BaseIndex = Vertices.Num();

		// Side ring of base vertices, plus apex copies (one per side for flat shading).
		for (int32 i = 0; i < Sides; ++i)
		{
			const float Angle = (2.0f * PI * i) / Sides;
			const FVector Radial = FMath::Cos(Angle) * Tangent + FMath::Sin(Angle) * Bitangent;
			const FVector BaseVert = BaseCenter + Radial * BaseRadius;
			const FVector SideNormal = (Radial - Axis * FVector::DotProduct(Radial, Axis)).GetSafeNormal();

			Vertices.Add(BaseVert);
			Normals.Add(SideNormal);
			UV0.Add(FVector2D(static_cast<float>(i) / Sides, 0.0f));
			Colors.Add(Color);

			Vertices.Add(Apex);
			Normals.Add(SideNormal);
			UV0.Add(FVector2D(static_cast<float>(i) / Sides, 1.0f));
			Colors.Add(Color);
		}

		for (int32 i = 0; i < Sides; ++i)
		{
			const int32 I0 = BaseIndex + i * 2;
			const int32 I1 = BaseIndex + i * 2 + 1;
			const int32 I2 = BaseIndex + ((i + 1) % Sides) * 2;

			Triangles.Add(I0);
			Triangles.Add(I1);
			Triangles.Add(I2);
		}

		// Base cap so the body doesn't show a hole where the beak meets it.
		const int32 CapCenterIndex = Vertices.Num();
		Vertices.Add(BaseCenter);
		Normals.Add(-Axis);
		UV0.Add(FVector2D(0.5f, 0.5f));
		Colors.Add(Color);

		const int32 CapRingStart = Vertices.Num();
		for (int32 i = 0; i < Sides; ++i)
		{
			const float Angle = (2.0f * PI * i) / Sides;
			const FVector Radial = FMath::Cos(Angle) * Tangent + FMath::Sin(Angle) * Bitangent;
			Vertices.Add(BaseCenter + Radial * BaseRadius);
			Normals.Add(-Axis);
			UV0.Add(FVector2D(0.5f + 0.5f * FMath::Cos(Angle), 0.5f + 0.5f * FMath::Sin(Angle)));
			Colors.Add(Color);
		}

		for (int32 i = 0; i < Sides; ++i)
		{
			Triangles.Add(CapCenterIndex);
			Triangles.Add(CapRingStart + ((i + 1) % Sides));
			Triangles.Add(CapRingStart + i);
		}
	}

	void EmitDoubleSidedTriangle(
		const FVector& V0,
		const FVector& V1,
		const FVector& V2,
		const FLinearColor& Color,
		TArray<FVector>& Vertices,
		TArray<int32>& Triangles,
		TArray<FVector>& Normals,
		TArray<FVector2D>& UV0,
		TArray<FLinearColor>& Colors)
	{
		const FVector FaceNormal = FVector::CrossProduct(V1 - V0, V2 - V0).GetSafeNormal();

		const int32 FrontBase = Vertices.Num();
		Vertices.Add(V0); Vertices.Add(V1); Vertices.Add(V2);
		Normals.Add(FaceNormal); Normals.Add(FaceNormal); Normals.Add(FaceNormal);
		UV0.Add(FVector2D(0.0f, 0.0f)); UV0.Add(FVector2D(1.0f, 0.0f)); UV0.Add(FVector2D(0.5f, 1.0f));
		Colors.Add(Color); Colors.Add(Color); Colors.Add(Color);
		Triangles.Add(FrontBase); Triangles.Add(FrontBase + 1); Triangles.Add(FrontBase + 2);

		const int32 BackBase = Vertices.Num();
		Vertices.Add(V0); Vertices.Add(V2); Vertices.Add(V1);
		Normals.Add(-FaceNormal); Normals.Add(-FaceNormal); Normals.Add(-FaceNormal);
		UV0.Add(FVector2D(0.0f, 0.0f)); UV0.Add(FVector2D(0.5f, 1.0f)); UV0.Add(FVector2D(1.0f, 0.0f));
		Colors.Add(Color); Colors.Add(Color); Colors.Add(Color);
		Triangles.Add(BackBase); Triangles.Add(BackBase + 1); Triangles.Add(BackBase + 2);
	}
}

AFlappyBird::AFlappyBird()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BodyMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(Root);

	WingMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("WingMesh"));
	WingMesh->SetupAttachment(BodyMesh);

	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));
	Collider->SetupAttachment(Root);
	Collider->SetSphereRadius(CollisionRadius);
	Collider->SetCollisionProfileName(TEXT("OverlapAll"));
	Collider->SetGenerateOverlapEvents(true);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Root);
	// Camera sits off to one side along +Y, looking back at the bird (-Y). Bird's +X (forward) reads as screen-right.
	Camera->SetRelativeLocation(FVector(0.0f, CameraDistance, 0.0f));
	Camera->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	// World-lock the camera: ignore the bird's transform so the bird can move up/down within the frame.
	// Classic Flappy feel — going off the top/bottom edge becomes a visible "you're about to die" signal.
	Camera->SetUsingAbsoluteLocation(true);
	Camera->SetUsingAbsoluteRotation(true);

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// BasicShapeMaterial ships with the engine and exposes a "Color" vector parameter we can drive via MIDs.
	// This is our path to per-part colors without authoring any material .uasset.
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicShapeMaterial(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BasicShapeMaterial.Succeeded())
	{
		ColoredParentMaterial = BasicShapeMaterial.Object;
	}
}

void AFlappyBird::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (Camera)
	{
		// With bAbsoluteLocation set in the constructor, these "relative" values are written into
		// the world transform directly. BeginPlay nudges them to follow the actual spawn X.
		Camera->SetRelativeLocation(FVector(0.0f, CameraDistance, 0.0f));
		Camera->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	}

	if (Collider)
	{
		Collider->SetSphereRadius(CollisionRadius);
	}

	BuildBody();
	BuildWing();
}

void AFlappyBird::BeginPlay()
{
	Super::BeginPlay();

	if (Camera)
	{
		// Pin the camera to the bird's spawn X (so it works even with a PlayerStart offset), out along +Y, height 0.
		const FVector ActorLoc = GetActorLocation();
		Camera->SetWorldLocation(FVector(ActorLoc.X, CameraDistance, 0.0f));
		Camera->SetWorldRotation(FRotator(0.0f, -90.0f, 0.0f));
	}
}

void AFlappyBird::BuildBody()
{
	if (!BodyMesh)
	{
		return;
	}

	BodyMesh->ClearAllMeshSections();

	const FLinearColor BodyColor(1.0f, 0.84f, 0.18f);
	const FLinearColor BeakColor(1.0f, 0.45f, 0.05f);
	const FLinearColor EyeWhite(0.96f, 0.96f, 0.96f);
	const FLinearColor EyePupil(0.04f, 0.04f, 0.04f);

	// Each body part is its own mesh section so we can give it its own MID + color.
	// BasicShapeMaterial ignores vertex colors, so per-section MIDs are how we get color at all.
	const TArray<FProcMeshTangent> Tangents;

	auto MakeSection = [&](int32 SectionIndex, const FLinearColor& Color, const TFunction<void(
		TArray<FVector>&, TArray<int32>&, TArray<FVector>&, TArray<FVector2D>&, TArray<FLinearColor>&)>& Build)
	{
		TArray<FVector> Vertices;
		TArray<int32> Triangles;
		TArray<FVector> Normals;
		TArray<FVector2D> UV0;
		TArray<FLinearColor> Colors;
		Build(Vertices, Triangles, Normals, UV0, Colors);
		if (Vertices.Num() == 0)
		{
			return;
		}
		BodyMesh->CreateMeshSection_LinearColor(SectionIndex, Vertices, Triangles, Normals, UV0, Colors, Tangents, false);
		ApplySectionColor(BodyMesh, SectionIndex, Color);
	};

	// Section 0 — body (stretched sphere centered at origin).
	MakeSection(0, BodyColor, [&](auto& V, auto& T, auto& N, auto& U, auto& C)
	{
		EmitSphere(FVector::ZeroVector, BodyRadii, SphereRings, SphereSegments, BodyColor, V, T, N, U, C);
	});

	// Section 1 — beak (cone pointing along +X).
	MakeSection(1, BeakColor, [&](auto& V, auto& T, auto& N, auto& U, auto& C)
	{
		const FVector BeakBase(BodyRadii.X * 0.88f, 0.0f, -BodyRadii.Z * 0.10f);
		const FVector BeakApex = BeakBase + FVector(BeakLength, 0.0f, 0.0f);
		EmitCone(BeakApex, BeakBase, BeakRadius, 10, BeakColor, V, T, N, U, C);
	});

	// Eye placed on the +Y (camera-facing) side, just outside the body skin so it reads as a bump rather than a buried dot.
	const FVector EyeCenter(BodyRadii.X * 0.55f, BodyRadii.Y * 1.0f, BodyRadii.Z * 0.45f);

	// Section 2 — eye white.
	MakeSection(2, EyeWhite, [&](auto& V, auto& T, auto& N, auto& U, auto& C)
	{
		EmitSphere(EyeCenter, FVector(EyeRadius, EyeRadius, EyeRadius), 8, 12, EyeWhite, V, T, N, U, C);
	});

	// Section 3 — eye pupil.
	MakeSection(3, EyePupil, [&](auto& V, auto& T, auto& N, auto& U, auto& C)
	{
		EmitSphere(EyeCenter + FVector(EyeRadius * 0.45f, EyeRadius * 0.55f, 0.0f),
			FVector(EyeRadius * 0.55f, EyeRadius * 0.55f, EyeRadius * 0.55f), 6, 10, EyePupil, V, T, N, U, C);
	});
}

void AFlappyBird::BuildWing()
{
	if (!WingMesh)
	{
		return;
	}

	WingMesh->ClearAllMeshSections();

	// Place the shoulder just outside the body's lateral skin so the wing has clear air to swing through —
	// previously it was tucked inside the body and the flap was barely visible.
	const FVector Shoulder(BodyRadii.X * 0.10f, BodyRadii.Y * 1.15f, BodyRadii.Z * 0.35f);
	WingMesh->SetRelativeLocation(Shoulder);

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FLinearColor> Colors;
	const TArray<FProcMeshTangent> Tangents;

	const FLinearColor WingColor(1.0f, 0.55f, 0.08f);

	// Triangle in the component's local X-Z plane (Y=0). Shoulder is at the origin.
	const FVector V0(0.0f, 0.0f, 0.0f);
	const FVector V1(-WingLength, 0.0f, WingTipHeight);
	const FVector V2(-WingLength * 0.85f, 0.0f, -WingTipHeight * 1.1f);

	EmitDoubleSidedTriangle(V0, V1, V2, WingColor, Vertices, Triangles, Normals, UV0, Colors);

	WingMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, Colors, Tangents, false);
	ApplySectionColor(WingMesh, 0, WingColor);
}

void AFlappyBird::ApplySectionColor(UProceduralMeshComponent* Mesh, int32 SectionIndex, const FLinearColor& Color)
{
	if (!Mesh || !ColoredParentMaterial)
	{
		return;
	}
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(ColoredParentMaterial, Mesh);
	if (!MID)
	{
		return;
	}
	MID->SetVectorParameterValue(TEXT("Color"), Color);
	Mesh->SetMaterial(SectionIndex, MID);
}

void AFlappyBird::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Gravity + clamped fall speed.
	VerticalVelocity -= Gravity * DeltaSeconds;
	VerticalVelocity = FMath::Max(VerticalVelocity, -MaxFallSpeed);

	FVector Location = GetActorLocation();
	Location.Z += VerticalVelocity * DeltaSeconds;
	Location.Y = 0.0f; // 2D plane lock.
	SetActorLocation(Location);

	// Wing flap — accumulate phase rather than reading off ElapsedTime, so the sine stays continuous
	// when the effective frequency jumps during a click-boost.
	const float BoostFactor = (FlapBoostDuration > 0.0f)
		? FMath::Clamp(FlapBoostRemaining / FlapBoostDuration, 0.0f, 1.0f)
		: 0.0f;
	const float EffectiveFreq = FlapFrequencyHz + FlapBoostFrequencyAddHz * BoostFactor;
	FlapPhase += DeltaSeconds * 2.0f * PI * EffectiveFreq;
	FlapBoostRemaining = FMath::Max(0.0f, FlapBoostRemaining - DeltaSeconds);

	if (WingMesh)
	{
		const float Pitch = FlapRestDeg + FlapAmplitudeDeg * FMath::Sin(FlapPhase);
		WingMesh->SetRelativeRotation(FRotator(Pitch, 0.0f, 0.0f));
	}
}

void AFlappyBird::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (!PlayerInputComponent)
	{
		return;
	}
	PlayerInputComponent->BindAction(TEXT("Flap"), IE_Pressed, this, &AFlappyBird::Flap);
}

void AFlappyBird::Flap()
{
	VerticalVelocity = FlapImpulse;
	FlapBoostRemaining = FlapBoostDuration;
}
