#include "FlappyBackground.h"

#include "Components/BoxComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "FlappyBirdConstants.h"
#include "FlappyNeonMaterials.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	using namespace FlappyBirdConstants;

	// Kollie meters → UE cm, plus the axis swap kollie (X, Y, Z) → UE (X, Z, Y).
	// Z (vertical) and Y (depth) swap; X (scroll axis) stays.
	FVector KollieToUE(float X, float YVert, float ZDepth)
	{
		return FVector(X * 100.0f, ZDepth * 100.0f, YVert * 100.0f);
	}

	// Static star positions copied 1:1 from kollie's FlappyBirdScene.kt Sky subtree.
	// `Scale` is the kollie-side scale; engine Sphere is 100 cm diameter, so the
	// component scale is just `Scale * 1.0` (sphere mesh, scaled directly).
	// Sky's parent offset (kollie z = -18 m) is baked into the depth component
	// here so the kollie numbers transfer verbatim.
	struct FStarSpec { float X; float Y; float Z; float Scale; };
	const FStarSpec GStars[] = {
		{ -10.0f,  6.5f, -18.0f + -1.0f, 0.18f },
		{  -8.2f,  3.8f, -18.0f +  0.5f, 0.12f },
		{  -5.0f,  5.6f, -18.0f + -2.5f, 0.16f },
		{  -2.5f,  7.0f, -18.0f +  1.0f, 0.14f },
		{   0.6f,  8.2f, -18.0f + -1.5f, 0.20f },
		{   3.4f,  6.4f, -18.0f +  0.0f, 0.13f },
		{   5.8f,  4.5f, -18.0f +  1.5f, 0.15f },
		{   8.0f,  6.8f, -18.0f + -2.0f, 0.18f },
		{  10.5f,  3.6f, -18.0f +  0.4f, 0.12f },
		{  12.0f,  5.4f, -18.0f + -1.0f, 0.16f },
		{  -6.5f,  2.4f, -18.0f +  2.0f, 0.10f },
		{   2.0f,  3.0f, -18.0f +  2.4f, 0.11f },
	};

	// Five mountain bases — kollie X, kollie Y (base sits at WorldFloorY + 0.2),
	// kollie Z (depth), and (BaseRadius, Height) in meters.
	struct FMountainSpec { float X; float ZDepth; float BaseRadiusM; float HeightM; };
	const FMountainSpec GMountains[] = {
		{ -9.0f, -18.0f + -2.0f, 3.6f, 4.5f },
		{ -3.5f, -18.0f + -1.0f, 3.0f, 3.6f },
		{  1.5f, -18.0f + -1.5f, 4.2f, 5.4f },
		{  6.5f, -18.0f + -2.5f, 3.4f, 4.0f },
		{ 10.5f, -18.0f + -1.0f, 3.8f, 4.6f },
	};

	// Pentagonal cone with a flat base. Five sides matches kollie's `mountainCone`
	// (segments = 5) and gives the visible pyramid silhouette in the screenshots —
	// a smooth engine Cone mesh would scale into a circular cone instead.
	void EmitPentagonalCone(
		const FVector& BaseCenter,
		float BaseRadius,
		float Height,
		const FLinearColor& Color,
		TArray<FVector>& Vertices,
		TArray<int32>& Triangles,
		TArray<FVector>& Normals,
		TArray<FVector2D>& UV0,
		TArray<FLinearColor>& Colors)
	{
		constexpr int32 Sides = 5;
		const FVector Apex = BaseCenter + FVector(0.0f, 0.0f, Height);
		const int32 BaseIndex = Vertices.Num();

		// Two verts per side (base + apex) with the side's flat-shaded normal —
		// faceted look matches the kollie reference.
		for (int32 i = 0; i < Sides; ++i)
		{
			const float A0 = (2.0f * PI * i) / Sides;
			const float A1 = (2.0f * PI * (i + 1)) / Sides;
			const FVector R0(FMath::Cos(A0), FMath::Sin(A0), 0.0f);
			const FVector R1(FMath::Cos(A1), FMath::Sin(A1), 0.0f);
			const FVector V0 = BaseCenter + R0 * BaseRadius;
			const FVector V1 = BaseCenter + R1 * BaseRadius;
			const FVector Midpoint = (V0 + V1) * 0.5f;
			const FVector ToApex = (Apex - Midpoint).GetSafeNormal();
			const FVector EdgeDir = (V1 - V0).GetSafeNormal();
			FVector FaceNormal = FVector::CrossProduct(EdgeDir, ToApex).GetSafeNormal();
			// Flip if it ended up pointing inward (toward the cone axis).
			const FVector OutwardHint = (Midpoint - BaseCenter).GetSafeNormal();
			if (FVector::DotProduct(FaceNormal, OutwardHint) < 0.0f)
			{
				FaceNormal = -FaceNormal;
			}

			Vertices.Add(V0); Vertices.Add(V1); Vertices.Add(Apex);
			Normals.Add(FaceNormal); Normals.Add(FaceNormal); Normals.Add(FaceNormal);
			UV0.Add(FVector2D(0.0f, 0.0f)); UV0.Add(FVector2D(1.0f, 0.0f)); UV0.Add(FVector2D(0.5f, 1.0f));
			Colors.Add(Color); Colors.Add(Color); Colors.Add(Color);

			Triangles.Add(BaseIndex + i * 3);
			Triangles.Add(BaseIndex + i * 3 + 2);
			Triangles.Add(BaseIndex + i * 3 + 1);
		}

		// Base cap — pentagon facing down so the bottom isn't a visible hole when
		// the camera happens to look up at the mountain row.
		const int32 CapCenterIndex = Vertices.Num();
		Vertices.Add(BaseCenter);
		Normals.Add(FVector(0, 0, -1));
		UV0.Add(FVector2D(0.5f, 0.5f));
		Colors.Add(Color);

		const int32 CapRingStart = Vertices.Num();
		for (int32 i = 0; i < Sides; ++i)
		{
			const float A = (2.0f * PI * i) / Sides;
			Vertices.Add(BaseCenter + FVector(FMath::Cos(A), FMath::Sin(A), 0.0f) * BaseRadius);
			Normals.Add(FVector(0, 0, -1));
			UV0.Add(FVector2D(0.5f + 0.5f * FMath::Cos(A), 0.5f + 0.5f * FMath::Sin(A)));
			Colors.Add(Color);
		}

		for (int32 i = 0; i < Sides; ++i)
		{
			Triangles.Add(CapCenterIndex);
			Triangles.Add(CapRingStart + i);
			Triangles.Add(CapRingStart + ((i + 1) % Sides));
		}
	}
}

AFlappyBackground::AFlappyBackground()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Floor + ceiling kill boxes. Mesh visible only via the ground plane;
	// ceiling is collider-only (matches the kollie scene's invisible Ceiling).
	auto MakeKillBox = [&](FName Name) -> UBoxComponent*
	{
		UBoxComponent* Box = CreateDefaultSubobject<UBoxComponent>(Name);
		Box->SetupAttachment(Root);
		Box->SetCollisionProfileName(TEXT("OverlapAll"));
		Box->SetGenerateOverlapEvents(true);
		return Box;
	};
	FloorCollider = MakeKillBox(TEXT("FloorCollider"));
	CeilingCollider = MakeKillBox(TEXT("CeilingCollider"));

	GroundMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundMesh"));
	GroundMesh->SetupAttachment(Root);
	GroundMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GroundMesh->SetCastShadow(false);

	MoonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MoonMesh"));
	MoonMesh->SetupAttachment(Root);
	MoonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MoonMesh->SetCastShadow(false);

	MountainsMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MountainsMesh"));
	MountainsMesh->SetupAttachment(Root);
	MountainsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MountainsMesh->SetCastShadow(false);

	MoonLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLight"));
	MoonLight->SetupAttachment(Root);
	// Slightly warm-cool moonlight — same triple as kollie's MoonLight color (0.55, 0.65, 1.0).
	MoonLight->SetLightColor(FLinearColor(0.55f, 0.65f, 1.0f));
	MoonLight->SetIntensity(1.4f);
	MoonLight->SetCastShadows(false);

	// Sky fill — purple wash on upward-facing surfaces (mountain slopes, top of
	// bird, top of pipes). kollie's ambientSky is (0.10, 0.08, 0.20); boosted
	// here to ~(0.50, 0.40, 0.90) so it reads against the dark backdrop. Rotation
	// is applied in OnConstruction (forward = down).
	SkyFillUpLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SkyFillUpLight"));
	SkyFillUpLight->SetupAttachment(Root);
	SkyFillUpLight->SetLightColor(FLinearColor(0.50f, 0.40f, 0.90f));
	SkyFillUpLight->SetIntensity(2.0f);
	SkyFillUpLight->SetCastShadows(false);

	// Ground fill — dimmer purple bounce on downward-facing surfaces (underside
	// of bird, underside of pipe caps). kollie's ambientGround is (0.03, 0.02,
	// 0.08); boosted to (0.20, 0.15, 0.40) so it's actually visible.
	SkyFillDownLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SkyFillDownLight"));
	SkyFillDownLight->SetupAttachment(Root);
	SkyFillDownLight->SetLightColor(FLinearColor(0.20f, 0.15f, 0.40f));
	SkyFillDownLight->SetIntensity(0.8f);
	SkyFillDownLight->SetCastShadows(false);

	// Stars — twelve fixed instances. Each is a small Sphere static mesh; we use
	// CreateDefaultSubobject so the components are introspectable in the editor.
	StarMeshes.Reserve(UE_ARRAY_COUNT(GStars));
	for (int32 i = 0; i < (int32)UE_ARRAY_COUNT(GStars); ++i)
	{
		const FName Name(*FString::Printf(TEXT("Star_%02d"), i));
		UStaticMeshComponent* Star = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Star->SetupAttachment(Root);
		Star->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Star->SetCastShadow(false);
		StarMeshes.Add(Star);
	}

	// Engine static meshes. Sphere is 100 cm diameter (50 cm radius);
	// Plane is 200 cm × 200 cm in the XY plane facing +Z.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (SphereMesh.Succeeded())
	{
		MoonMesh->SetStaticMesh(SphereMesh.Object);
		for (UStaticMeshComponent* S : StarMeshes)
		{
			S->SetStaticMesh(SphereMesh.Object);
		}
	}
	if (PlaneMesh.Succeeded())
	{
		GroundMesh->SetStaticMesh(PlaneMesh.Object);
	}
}

void AFlappyBackground::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// --- Ground (single huge plane at Z=floor). Plane.Plane is 200 cm; scale 60 → 12000 cm side.
	if (GroundMesh)
	{
		// Plane normal faces +Z by default — perfect for a horizontal ground at z = floor.
		GroundMesh->SetRelativeLocation(FVector(0.0f, 0.0f, WorldFloorZ - 5.0f));
		GroundMesh->SetRelativeScale3D(FVector(60.0f, 60.0f, 1.0f));
		if (UMaterialInstanceDynamic* MID = FlappyNeonMaterials::MakeMID(GroundMesh, FlappyNeonPalette::Ground))
		{
			GroundMesh->SetMaterial(0, MID);
		}
	}

	// --- Floor / ceiling colliders (oversized in X and Y, thin in Z).
	if (FloorCollider)
	{
		FloorCollider->SetRelativeLocation(FVector(0.0f, 0.0f, WorldFloorZ - 50.0f));
		FloorCollider->SetBoxExtent(FVector(6000.0f, 3000.0f, 50.0f), /*bUpdateOverlaps=*/true);
	}
	if (CeilingCollider)
	{
		CeilingCollider->SetRelativeLocation(FVector(0.0f, 0.0f, WorldCeilingZ + 50.0f));
		CeilingCollider->SetBoxExtent(FVector(6000.0f, 3000.0f, 50.0f), true);
	}

	// --- Moon. kollie position (-7.5, 4.5, -3) under Sky parent (0, 0, -18) → world (-7.5, 4.5, -21).
	if (MoonMesh)
	{
		MoonMesh->SetRelativeLocation(KollieToUE(-7.5f, 4.5f, -18.0f + -3.0f));
		// Sphere mesh is 100 cm; scale 2.4 → 240 cm diameter, matches kollie.
		MoonMesh->SetRelativeScale3D(FVector(2.4f));
		if (UMaterialInstanceDynamic* MID = FlappyNeonMaterials::MakeMID(MoonMesh, FlappyNeonPalette::Moon))
		{
			MoonMesh->SetMaterial(0, MID);
		}
	}

	// --- Stars.
	for (int32 i = 0; i < (int32)UE_ARRAY_COUNT(GStars) && i < StarMeshes.Num(); ++i)
	{
		const FStarSpec& Spec = GStars[i];
		UStaticMeshComponent* Star = StarMeshes[i];
		if (!Star) continue;
		Star->SetRelativeLocation(KollieToUE(Spec.X, Spec.Y, Spec.Z));
		Star->SetRelativeScale3D(FVector(Spec.Scale));
		if (UMaterialInstanceDynamic* MID = FlappyNeonMaterials::MakeMID(Star, FlappyNeonPalette::Star))
		{
			Star->SetMaterial(0, MID);
		}
	}

	// --- Mountains (procedural mesh, single section, pentagonal cone for each).
	BuildMountainsMesh();

	// --- Moonlight direction. kollie (-0.4, -1.0, -0.3) in (X, Yvert, Zdepth) →
	// UE (-0.4, -0.3, -1.0). Rotation is built from that forward vector.
	if (MoonLight)
	{
		const FVector Forward = FVector(-0.4f, -0.3f, -1.0f).GetSafeNormal();
		MoonLight->SetRelativeRotation(Forward.Rotation());
	}

	// --- Sky fill lights. Forward = down → upward-facing surfaces get the sky tint.
	// Forward = up → downward-facing surfaces get the dim ground tint.
	if (SkyFillUpLight)
	{
		SkyFillUpLight->SetRelativeRotation(FVector(0.0f, 0.0f, -1.0f).Rotation());
	}
	if (SkyFillDownLight)
	{
		SkyFillDownLight->SetRelativeRotation(FVector(0.0f, 0.0f, 1.0f).Rotation());
	}
}

void AFlappyBackground::BuildMountainsMesh()
{
	if (!MountainsMesh)
	{
		return;
	}
	MountainsMesh->ClearAllMeshSections();

	TArray<FVector> V;
	TArray<int32>   T;
	TArray<FVector> N;
	TArray<FVector2D> U;
	TArray<FLinearColor> C;

	for (const FMountainSpec& M : GMountains)
	{
		const FVector Base = KollieToUE(M.X, /*kollie Y*/ -4.5f + 0.2f /* floor + 0.2 */, M.ZDepth);
		EmitPentagonalCone(
			Base,
			M.BaseRadiusM * 100.0f,
			M.HeightM    * 100.0f,
			FlappyNeonPalette::Mountain.BaseColor,
			V, T, N, U, C);
	}

	const TArray<FProcMeshTangent> Tangents;
	MountainsMesh->CreateMeshSection_LinearColor(0, V, T, N, U, C, Tangents, /*bCreateCollision=*/false);
	FlappyNeonMaterials::ApplySectionSpec(MountainsMesh, 0, FlappyNeonPalette::Mountain);
}
