#include "FlappyBirdGameMode.h"

#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "FlappyBackground.h"
#include "FlappyBird.h"
#include "FlappyBirdHUD.h"
#include "Kismet/GameplayStatics.h"
#include "PipeSpawner.h"
#include "Sound/SoundBase.h"

AFlappyBirdGameMode::AFlappyBirdGameMode()
{
	DefaultPawnClass = AFlappyBird::StaticClass();
	HUDClass = AFlappyBirdHUD::StaticClass();
	BackgroundClass = AFlappyBackground::StaticClass();
	PipeSpawnerClass = APipeSpawner::StaticClass();
}

void AFlappyBirdGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World) return;

	if (BackgroundClass)
	{
		const FTransform At(FRotator::ZeroRotator, FVector::ZeroVector);
		Background = World->SpawnActor<AFlappyBackground>(BackgroundClass, At);
	}
	if (PipeSpawnerClass)
	{
		const FTransform At(FRotator::ZeroRotator, FVector::ZeroVector);
		PipeSpawner = World->SpawnActor<APipeSpawner>(PipeSpawnerClass, At);
	}

	// BGM — persistent across the whole session at 0.35 volume (kollie default).
	if (USoundBase* Bgm = LoadObject<USoundBase>(nullptr, TEXT("/Agility/Audio/Music/Herd_The_Stars_v2.Herd_The_Stars_v2")))
	{
		BgmComponent = UGameplayStatics::SpawnSound2D(
			this, Bgm,
			/*VolumeMultiplier=*/0.35f, /*PitchMultiplier=*/1.0f,
			/*StartTime=*/0.0f, /*ConcurrencySettings=*/nullptr,
			/*bPersistAcrossLevelTransition=*/true,
			/*bAutoDestroy=*/false);
		if (BgmComponent)
		{
			// Loop. SpawnSound2D doesn't loop by default; toggle via the sound asset's settings or set here.
			// Modifying loop on the component isn't a single-call API — for now we rely on the
			// imported SoundWave's "Looping" property being enabled. See Part 6 handoff for details.
		}
	}

	SfxScore = LoadObject<USoundBase>(nullptr, TEXT("/Agility/Audio/SFX/space-shooter/sfx_twoTone.sfx_twoTone"));
}

void AFlappyBirdGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BgmComponent)
	{
		BgmComponent->Stop();
		BgmComponent = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void AFlappyBirdGameMode::BeginPlayPhase()
{
	if (Phase != EFlappyPhase::Ready) return;
	Phase = EFlappyPhase::Playing;
	Score = 0;
}

void AFlappyBirdGameMode::NotifyBirdDied()
{
	if (Phase != EFlappyPhase::Playing) return;
	Phase = EFlappyPhase::Dead;
	if (Score > Best)
	{
		Best = Score;
	}
}

void AFlappyBirdGameMode::NotifyBirdScored()
{
	if (Phase != EFlappyPhase::Playing) return;
	++Score;
	if (SfxScore)
	{
		UGameplayStatics::PlaySound2D(this, SfxScore, 0.55f);
	}
}

void AFlappyBirdGameMode::NotifyBirdHit(APipePair* /*Pipe*/)
{
	if (Phase != EFlappyPhase::Playing) return;
	// Route through the bird so it plays the hit SFX + transitions to its Dead
	// state. The bird then calls NotifyBirdDied back into us. This keeps the
	// "bird drives its own death animation" responsibility clean.
	UWorld* World = GetWorld();
	if (!World) return;
	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (AFlappyBird* Bird = Cast<AFlappyBird>(PC->GetPawn()))
		{
			// Reuse Bird's Die() path. It's private — but we declared GameMode a friend.
			Bird->Die();
		}
	}
}

void AFlappyBirdGameMode::RequestRestart()
{
	if (PipeSpawner) PipeSpawner->ClearActivePipes();
	Phase = EFlappyPhase::Ready;
	Score = 0;

	UWorld* World = GetWorld();
	if (!World) return;
	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (AFlappyBird* Bird = Cast<AFlappyBird>(PC->GetPawn()))
		{
			Bird->ResetForReady();
		}
	}
}
