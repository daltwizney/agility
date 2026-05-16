#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PipePair.generated.h"

class UProceduralMeshComponent;
class USceneComponent;
class UBoxComponent;
class UMaterialInterface;
class UPrimitiveComponent;

UCLASS()
class AGILITY_API APipePair : public AActor
{
	GENERATED_BODY()

public:
	APipePair();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category = "Pipe")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Pipe")
	TObjectPtr<UProceduralMeshComponent> BottomPipeMesh;

	UPROPERTY(VisibleAnywhere, Category = "Pipe")
	TObjectPtr<UProceduralMeshComponent> TopPipeMesh;

	UPROPERTY(VisibleAnywhere, Category = "Pipe|Collision")
	TObjectPtr<UBoxComponent> BottomHitBox;

	UPROPERTY(VisibleAnywhere, Category = "Pipe|Collision")
	TObjectPtr<UBoxComponent> TopHitBox;

	UPROPERTY(VisibleAnywhere, Category = "Pipe|Collision")
	TObjectPtr<UBoxComponent> ScoreTrigger;

	// --- Shape (cm)
	UPROPERTY(EditAnywhere, Category = "Pipe|Shape")
	float GapCenterZ = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Pipe|Shape")
	float GapHeight = 280.0f;

	UPROPERTY(EditAnywhere, Category = "Pipe|Shape")
	float PipeRadius = 75.0f;

	UPROPERTY(EditAnywhere, Category = "Pipe|Shape")
	float RimRadius = 92.0f;

	UPROPERTY(EditAnywhere, Category = "Pipe|Shape")
	float RimHeight = 40.0f;

	UPROPERTY(EditAnywhere, Category = "Pipe|Shape")
	float PipeExtent = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Pipe|Shape", meta = (ClampMin = "6", ClampMax = "48"))
	int32 Sides = 18;

	UPROPERTY(EditAnywhere, Category = "Pipe|Shape")
	FLinearColor PipeColor = FLinearColor(0.18f, 0.65f, 0.20f);

	UPROPERTY(EditAnywhere, Category = "Pipe|Shape")
	FLinearColor RimColor = FLinearColor(0.12f, 0.50f, 0.16f);

	// --- Motion (cm/s and cm)
	UPROPERTY(EditAnywhere, Category = "Pipe|Motion")
	float ScrollSpeed = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Pipe|Motion")
	float DespawnX = -2000.0f;

	UFUNCTION()
	void OnHitBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnScoreTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void BuildPipes();
	void ApplySectionColor(UProceduralMeshComponent* Mesh, int32 SectionIndex, const FLinearColor& Color);

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ColoredParentMaterial;

	bool bScored = false;
};
