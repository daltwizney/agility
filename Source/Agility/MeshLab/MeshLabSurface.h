#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeshLabSurface.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EMeshLabShape : uint8
{
	Quad UMETA(DisplayName = "Quad"),
	Cube UMETA(DisplayName = "Cube"),
};

UCLASS()
class AGILITY_API AMeshLabSurface : public AActor
{
	GENERATED_BODY()

public:
	AMeshLabSurface();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, Category = "MeshLab")
	TObjectPtr<UProceduralMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, Category = "MeshLab|Shape")
	EMeshLabShape Shape = EMeshLabShape::Quad;

	UPROPERTY(EditAnywhere, Category = "MeshLab|Shape", meta = (ClampMin = "1.0"))
	float Size = 200.0f;

	UPROPERTY(EditAnywhere, Category = "MeshLab|Material")
	TObjectPtr<UMaterialInterface> Material;

	void Regenerate();

	// Static geometry helpers — exposed so any actor that owns its own UProceduralMeshComponent
	// (e.g. AMeshLabScene) can build the same shape without instantiating an AMeshLabSurface.
	static void BuildQuad(float Size, TArray<FVector>& Vertices, TArray<int32>& Triangles,
		TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& Colors);
	static void BuildCube(float Size, TArray<FVector>& Vertices, TArray<int32>& Triangles,
		TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& Colors);
};
