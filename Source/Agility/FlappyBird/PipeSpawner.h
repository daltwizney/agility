#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PipeSpawner.generated.h"

class APipePair;
class USceneComponent;

UCLASS()
class AGILITY_API APipeSpawner : public AActor
{
	GENERATED_BODY()

public:
	APipeSpawner();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, Category = "Spawner")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<APipePair> PipeClass;

	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "0.1"))
	float SpawnInterval = 1.6f;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	float InitialDelay = 1.0f;

	// World Z values defining the vertical range of the gap center.
	UPROPERTY(EditAnywhere, Category = "Spawner")
	float GapMinZ = -300.0f;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	float GapMaxZ = 300.0f;

	// Forwarded to each spawned pipe so the spawner is the single source of truth.
	UPROPERTY(EditAnywhere, Category = "Spawner|Pipe")
	float GapHeight = 280.0f;

	UPROPERTY(EditAnywhere, Category = "Spawner|Pipe")
	float ScrollSpeed = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Spawner|Pipe")
	float DespawnX = -2000.0f;

private:
	void SpawnPipe();

	FTimerHandle SpawnTimer;
};
