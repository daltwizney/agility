#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PipeSpawner.generated.h"

class APipePair;
class USceneComponent;

// Tick-driven spawner: every PipeSpawnInterval seconds while the game is in the
// PLAYING phase, spawn a new APipePair at PipeSpawnX with a random gap Z and
// alternating cyan/magenta colors. Game mode spawns this actor on BeginPlay,
// so the level itself stays empty. Game mode also calls ClearPipes() on
// restart to wipe any in-flight pipes.
UCLASS()
class AGILITY_API APipeSpawner : public AActor
{
	GENERATED_BODY()

public:
	APipeSpawner();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category = "Spawner")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<APipePair> PipeClass;

	// Destroys every pipe this spawner has ever produced that is still alive.
	// Called by the game mode on restart so dead-fall frames don't see leftover
	// pipes from the previous run.
	void ClearActivePipes();

private:
	void SpawnPipe();

	float SpawnTimer = 0.0f;
	int32 SpawnCounter = 0;
	TArray<TWeakObjectPtr<APipePair>> ActivePipes;
};
