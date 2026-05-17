#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PipePair.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;

// One cyan-or-magenta pipe pair: a top cube shaft + top cap, a bottom cube shaft
// + bottom cap, two kill colliders (one per column), and a thin score-trigger
// AABB centered in the gap. Mirrors kollie's `spawnPipePair` exactly, with the
// per-column "cap at gap end" wider-than-shaft block giving it the mario-pipe
// silhouette.
//
// Properties set by APipeSpawner before FinishSpawning():
//   - GapCenterZ : random Z in ±PipeGapZRange
//   - bUseCyan   : alternated each spawn so the playfield reads as motion even
//                  when nothing's animating
//
// Tick scrolls the actor in -X at PipeScrollSpeed and self-destructs once it
// crosses the despawn line.
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
	TObjectPtr<UStaticMeshComponent> TopShaftMesh;

	UPROPERTY(VisibleAnywhere, Category = "Pipe")
	TObjectPtr<UStaticMeshComponent> TopCapMesh;

	UPROPERTY(VisibleAnywhere, Category = "Pipe")
	TObjectPtr<UStaticMeshComponent> BottomShaftMesh;

	UPROPERTY(VisibleAnywhere, Category = "Pipe")
	TObjectPtr<UStaticMeshComponent> BottomCapMesh;

	UPROPERTY(VisibleAnywhere, Category = "Pipe|Collision")
	TObjectPtr<UBoxComponent> TopHitBox;

	UPROPERTY(VisibleAnywhere, Category = "Pipe|Collision")
	TObjectPtr<UBoxComponent> BottomHitBox;

	UPROPERTY(VisibleAnywhere, Category = "Pipe|Collision")
	TObjectPtr<UBoxComponent> ScoreTrigger;

	// Spawner-set: Z (vertical) of the gap center.
	UPROPERTY(EditAnywhere, Category = "Pipe")
	float GapCenterZ = 0.0f;

	// Spawner-set: alternates each spawn so adjacent pipes have different neon hues.
	UPROPERTY(EditAnywhere, Category = "Pipe")
	bool bUseCyan = true;

	UFUNCTION()
	void OnHitBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnScoreTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void BuildPipes();

	bool bScored = false;
};
