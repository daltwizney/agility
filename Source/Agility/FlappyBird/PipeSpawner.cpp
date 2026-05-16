#include "PipeSpawner.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "PipePair.h"
#include "TimerManager.h"

APipeSpawner::APipeSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PipeClass = APipePair::StaticClass();
}

void APipeSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!GetWorld() || !PipeClass)
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		SpawnTimer,
		this,
		&APipeSpawner::SpawnPipe,
		SpawnInterval,
		/*bLoop=*/true,
		/*FirstDelay=*/InitialDelay);
}

void APipeSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(SpawnTimer);
	}
	Super::EndPlay(EndPlayReason);
}

void APipeSpawner::SpawnPipe()
{
	UWorld* World = GetWorld();
	if (!World || !PipeClass)
	{
		return;
	}

	const float GapZ = FMath::FRandRange(FMath::Min(GapMinZ, GapMaxZ), FMath::Max(GapMinZ, GapMaxZ));

	// Pipes spawn at the spawner's X (their entry point) on the Y=0 plane.
	const FVector SpawnLocation(GetActorLocation().X, 0.0f, 0.0f);
	const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

	// Deferred spawn so the pipe builds its mesh with the right gap Z during OnConstruction
	// (which fires inside FinishSpawning, after we've set the gap properties).
	APipePair* Pipe = World->SpawnActorDeferred<APipePair>(PipeClass, SpawnTransform);
	if (!Pipe)
	{
		return;
	}
	Pipe->GapCenterZ = GapZ;
	Pipe->GapHeight = GapHeight;
	Pipe->ScrollSpeed = ScrollSpeed;
	Pipe->DespawnX = DespawnX;
	Pipe->FinishSpawning(SpawnTransform);
}
