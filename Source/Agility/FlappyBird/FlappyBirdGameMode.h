#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FlappyBirdGameMode.generated.h"

class APipePair;
class APipeSpawner;
class AFlappyBackground;
class UAudioComponent;

UENUM()
enum class EFlappyPhase : uint8
{
	Ready,    // First tap will start the game.
	Playing,  // Pipes scroll, bird flies. Hit a pipe / floor / ceiling → Dead.
	Dead,     // Pipes frozen; next tap restarts.
};

// Owns the FlappyBird run lifecycle: spawns the background + pipe spawner on
// BeginPlay, tracks the phase machine (Ready → Playing → Dead), tallies score
// and best, and routes restart back to the bird + spawner.
//
// The level can be entirely empty — drop into PIE with this game mode as the
// World Settings override and the GameMode will conjure everything it needs.
UCLASS()
class AGILITY_API AFlappyBirdGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFlappyBirdGameMode();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	EFlappyPhase GetPhase() const { return Phase; }
	bool IsPlaying()       const { return Phase == EFlappyPhase::Playing; }
	int32 GetScore()       const { return Score; }
	int32 GetBest()        const { return Best; }

	// Called by the bird on first Flap input while in Ready.
	void BeginPlayPhase();

	// Called by the bird when it dies (pipe / floor / ceiling overlap).
	void NotifyBirdDied();

	// Called by APipePair when the bird crosses a score gate.
	void NotifyBirdScored();

	// Called by APipePair when the bird overlaps a pipe column.
	void NotifyBirdHit(APipePair* Pipe);

	// Called by the bird on tap-while-Dead.
	void RequestRestart();

	// Classes the GameMode spawns on BeginPlay. Exposed so a tutorial reader
	// can subclass and swap them in their host project if they want to extend
	// the demo.
	UPROPERTY(EditAnywhere, Category = "FlappyBird")
	TSubclassOf<AFlappyBackground> BackgroundClass;

	UPROPERTY(EditAnywhere, Category = "FlappyBird")
	TSubclassOf<APipeSpawner> PipeSpawnerClass;

private:
	UPROPERTY() TObjectPtr<AFlappyBackground> Background;
	UPROPERTY() TObjectPtr<APipeSpawner> PipeSpawner;
	UPROPERTY() TObjectPtr<UAudioComponent> BgmComponent;
	UPROPERTY() TObjectPtr<class USoundBase> SfxScore;

	EFlappyPhase Phase = EFlappyPhase::Ready;
	int32 Score = 0;
	int32 Best = 0;
};
