#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralFractalTree.generated.h"

class UProceduralMeshComponent;
struct FRandomStream;

UCLASS()
class AGILITY_API AProceduralFractalTree : public AActor
{
	GENERATED_BODY()

public:
	AProceduralFractalTree();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, Category = "Tree")
	TObjectPtr<UProceduralMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, Category = "Tree|Shape", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxDepth = 6;

	UPROPERTY(EditAnywhere, Category = "Tree|Shape", meta = (ClampMin = "1.0"))
	float TrunkLength = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Tree|Shape", meta = (ClampMin = "0.1"))
	float TrunkRadius = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Tree|Shape", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float LengthDecay = 0.75f;

	UPROPERTY(EditAnywhere, Category = "Tree|Shape", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float RadiusDecay = 0.65f;

	UPROPERTY(EditAnywhere, Category = "Tree|Shape", meta = (ClampMin = "1", ClampMax = "6"))
	int32 BranchesPerNode = 3;

	UPROPERTY(EditAnywhere, Category = "Tree|Shape", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float BranchAngle = 35.0f;

	UPROPERTY(EditAnywhere, Category = "Tree|Shape", meta = (ClampMin = "0.0", ClampMax = "45.0"))
	float AngleJitter = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Tree|Mesh", meta = (ClampMin = "3", ClampMax = "16"))
	int32 SidesPerBranch = 6;

	UPROPERTY(EditAnywhere, Category = "Tree")
	int32 Seed = 7;

private:
	void Regenerate();

	void Branch(const FVector& Start, const FVector& Direction, float Length, float Radius, int32 Depth, FRandomStream& Rng,
		TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& Colors) const;

	void EmitCylinder(const FVector& Start, const FVector& End, float StartRadius, float EndRadius, int32 Sides, const FLinearColor& BaseColor,
		TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& Colors) const;
};
