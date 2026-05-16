#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralSpiralGalaxy.generated.h"

class UHierarchicalInstancedStaticMeshComponent;

UCLASS()
class AGILITY_API AProceduralSpiralGalaxy : public AActor
{
	GENERATED_BODY()

public:
	AProceduralSpiralGalaxy();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, Category = "Galaxy")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> InstancedMeshes;

	UPROPERTY(EditAnywhere, Category = "Galaxy|Shape", meta = (ClampMin = "1", ClampMax = "100000"))
	int32 NumStars = 5000;

	UPROPERTY(EditAnywhere, Category = "Galaxy|Shape", meta = (ClampMin = "1", ClampMax = "16"))
	int32 NumArms = 4;

	UPROPERTY(EditAnywhere, Category = "Galaxy|Shape", meta = (ClampMin = "1.0"))
	float DiskRadius = 5000.0f;

	UPROPERTY(EditAnywhere, Category = "Galaxy|Shape", meta = (ClampMin = "0.1"))
	float SpiralTurns = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Galaxy|Shape", meta = (ClampMin = "0.0"))
	float ArmJitter = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Galaxy|Shape", meta = (ClampMin = "0.0"))
	float ZJitter = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Galaxy|Shape", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float CoreBias = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Galaxy|Stars", meta = (ClampMin = "1.0"))
	float StarSizeMin = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Galaxy|Stars", meta = (ClampMin = "1.0"))
	float StarSizeMax = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Galaxy")
	int32 Seed = 42;

private:
	void Regenerate();
};
