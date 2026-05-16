#include "ProceduralSpiralGalaxy.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Math/RandomStream.h"
#include "UObject/ConstructorHelpers.h"

AProceduralSpiralGalaxy::AProceduralSpiralGalaxy()
{
	PrimaryActorTick.bCanEverTick = false;

	InstancedMeshes = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("InstancedMeshes"));
	SetRootComponent(InstancedMeshes);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		InstancedMeshes->SetStaticMesh(CubeMeshFinder.Object);
	}
}

void AProceduralSpiralGalaxy::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Regenerate();
}

void AProceduralSpiralGalaxy::Regenerate()
{
	if (!InstancedMeshes || !InstancedMeshes->GetStaticMesh())
	{
		return;
	}

	InstancedMeshes->ClearInstances();

	FRandomStream Rng(Seed);

	constexpr float CubeBaseSize = 100.0f;
	const int32 ClampedArms = FMath::Max(1, NumArms);
	const float ArmAngleStep = 2.0f * PI / ClampedArms;

	TArray<FTransform> InstanceTransforms;
	InstanceTransforms.Reserve(NumStars);

	for (int32 i = 0; i < NumStars; ++i)
	{
		// Power-distribute T in [0,1] along the arm; CoreBias > 1 packs stars toward the core.
		const float U = Rng.FRand();
		const float T = FMath::Pow(U, CoreBias);

		const int32 ArmIndex = Rng.RandRange(0, ClampedArms - 1);
		const float BaseAngle = ArmIndex * ArmAngleStep;
		const float Theta = BaseAngle + T * SpiralTurns * 2.0f * PI;
		const float Radius = T * DiskRadius;

		const float X = Radius * FMath::Cos(Theta);
		const float Y = Radius * FMath::Sin(Theta);

		const float JitterX = Rng.FRandRange(-ArmJitter, ArmJitter);
		const float JitterY = Rng.FRandRange(-ArmJitter, ArmJitter);
		const float JitterZ = Rng.FRandRange(-ZJitter, ZJitter);

		const float Size = FMath::Lerp(StarSizeMax, StarSizeMin, T);
		const FVector Scale(Size / CubeBaseSize);

		const FVector Loc(X + JitterX, Y + JitterY, JitterZ);
		InstanceTransforms.Emplace(FQuat::Identity, Loc, Scale);
	}

	InstancedMeshes->AddInstances(InstanceTransforms, /*bShouldReturnIndices=*/false);
}
