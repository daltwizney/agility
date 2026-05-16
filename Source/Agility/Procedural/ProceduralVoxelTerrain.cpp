#include "ProceduralVoxelTerrain.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Math/RandomStream.h"
#include "UObject/ConstructorHelpers.h"

AProceduralVoxelTerrain::AProceduralVoxelTerrain()
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

void AProceduralVoxelTerrain::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Regenerate();
}

float AProceduralVoxelTerrain::SampleHeight(float X, float Y, float OffX, float OffY) const
{
	float Total = 0.0f;
	float Amplitude = 1.0f;
	float Frequency = 1.0f;
	float MaxAmp = 0.0f;

	for (int32 i = 0; i < Octaves; ++i)
	{
		const float SampleX = (X + OffX) * NoiseScale * Frequency;
		const float SampleY = (Y + OffY) * NoiseScale * Frequency;
		Total += FMath::PerlinNoise2D(FVector2D(SampleX, SampleY)) * Amplitude;
		MaxAmp += Amplitude;
		Amplitude *= Persistence;
		Frequency *= Lacunarity;
	}

	return (MaxAmp > 0.0f) ? (Total / MaxAmp) : 0.0f;
}

void AProceduralVoxelTerrain::Regenerate()
{
	if (!InstancedMeshes || !InstancedMeshes->GetStaticMesh())
	{
		return;
	}

	InstancedMeshes->ClearInstances();

	FRandomStream Rng(Seed);
	const float OffX = Rng.FRandRange(-10000.0f, 10000.0f);
	const float OffY = Rng.FRandRange(-10000.0f, 10000.0f);

	// Engine cube primitive is 100 cm on each side; rescale so cubes tile cleanly at CellSize.
	constexpr float CubeBaseSize = 100.0f;
	const FVector InstanceScale(CellSize / CubeBaseSize);

	TArray<FTransform> InstanceTransforms;
	InstanceTransforms.Reserve(GridSizeX * GridSizeY);

	for (int32 ix = 0; ix < GridSizeX; ++ix)
	{
		for (int32 iy = 0; iy < GridSizeY; ++iy)
		{
			const float NormHeight = SampleHeight(static_cast<float>(ix), static_cast<float>(iy), OffX, OffY);
			const float WorldHeight = NormHeight * HeightScale;
			const float WorldX = ix * CellSize;
			const float WorldY = iy * CellSize;

			if (bStackVertically)
			{
				const int32 TopCell = FMath::FloorToInt(WorldHeight / CellSize);
				const int32 LowestCell = FMath::Min(0, TopCell);
				for (int32 iz = LowestCell; iz <= TopCell; ++iz)
				{
					InstanceTransforms.Emplace(FQuat::Identity, FVector(WorldX, WorldY, iz * CellSize), InstanceScale);
				}
			}
			else
			{
				InstanceTransforms.Emplace(FQuat::Identity, FVector(WorldX, WorldY, WorldHeight), InstanceScale);
			}
		}
	}

	InstancedMeshes->AddInstances(InstanceTransforms, /*bShouldReturnIndices=*/false);
}
