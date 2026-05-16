#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralVoxelTerrain.generated.h"

class UHierarchicalInstancedStaticMeshComponent;

UCLASS()
class AGILITY_API AProceduralVoxelTerrain : public AActor
{
	GENERATED_BODY()

public:
	AProceduralVoxelTerrain();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> InstancedMeshes;

	UPROPERTY(EditAnywhere, Category = "Terrain|Grid", meta = (ClampMin = "1", ClampMax = "512"))
	int32 GridSizeX = 64;

	UPROPERTY(EditAnywhere, Category = "Terrain|Grid", meta = (ClampMin = "1", ClampMax = "512"))
	int32 GridSizeY = 64;

	UPROPERTY(EditAnywhere, Category = "Terrain|Grid", meta = (ClampMin = "1.0"))
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Terrain|Noise", meta = (ClampMin = "0.0001"))
	float NoiseScale = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Terrain|Noise")
	float HeightScale = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Terrain|Noise", meta = (ClampMin = "1", ClampMax = "8"))
	int32 Octaves = 4;

	UPROPERTY(EditAnywhere, Category = "Terrain|Noise", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Persistence = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Terrain|Noise", meta = (ClampMin = "1.0"))
	float Lacunarity = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Terrain|Noise")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, Category = "Terrain|Style")
	bool bStackVertically = false;

private:
	void Regenerate();
	float SampleHeight(float X, float Y, float OffX, float OffY) const;
};
