#include "PipeSpawner.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "FlappyBirdConstants.h"
#include "FlappyBirdGameMode.h"
#include "PipePair.h"

using namespace FlappyBirdConstants;

APipeSpawner::APipeSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PipeClass = APipePair::StaticClass();

	// Brief grace before the first pipe so the player has a beat after their
	// initial flap.
	SpawnTimer = InitialGraceDelay;
}

void APipeSpawner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UWorld* World = GetWorld();
	const AFlappyBirdGameMode* GM = World ? World->GetAuthGameMode<AFlappyBirdGameMode>() : nullptr;
	if (!GM || !GM->IsPlaying())
	{
		// While READY/DEAD, keep the timer parked at the grace delay so the
		// first PLAYING tick fires the same beat as a fresh game would.
		SpawnTimer = InitialGraceDelay;
		return;
	}

	SpawnTimer -= DeltaSeconds;
	if (SpawnTimer <= 0.0f)
	{
		SpawnPipe();
		SpawnTimer = PipeSpawnInterval;
	}
}

void APipeSpawner::SpawnPipe()
{
	UWorld* World = GetWorld();
	if (!World || !PipeClass)
	{
		return;
	}

	const float GapZ = FMath::FRandRange(-PipeGapZRange, PipeGapZRange);
	const FVector SpawnLocation(PipeSpawnX, 0.0f, 0.0f);
	const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

	// Deferred spawn so the pipe's OnConstruction (which builds geometry) sees
	// the right GapCenterZ + cyan/magenta flag.
	APipePair* Pipe = World->SpawnActorDeferred<APipePair>(PipeClass, SpawnTransform);
	if (!Pipe)
	{
		return;
	}
	Pipe->GapCenterZ = GapZ;
	Pipe->bUseCyan = (SpawnCounter % 2 == 0);
	Pipe->FinishSpawning(SpawnTransform);

	++SpawnCounter;
	ActivePipes.Add(Pipe);

	// Cheap GC of dead entries — keeps the array bounded across long runs.
	ActivePipes.RemoveAll([](const TWeakObjectPtr<APipePair>& P) { return !P.IsValid(); });
}

void APipeSpawner::ClearActivePipes()
{
	for (const TWeakObjectPtr<APipePair>& P : ActivePipes)
	{
		if (APipePair* Pipe = P.Get())
		{
			Pipe->Destroy();
		}
	}
	ActivePipes.Reset();
	SpawnTimer = InitialGraceDelay;
}
