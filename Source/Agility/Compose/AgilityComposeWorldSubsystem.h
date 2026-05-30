#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AgilityComposeWorldSubsystem.generated.h"

// Spawns AAgilityComposeCounterActor into every game world on begin play, so the Compose overlay has
// an actor to drive without any editor placement or game-mode wiring. Auto-runs in PIE and in the
// packaged Android build alike.
UCLASS()
class AGILITY_API UAgilityComposeWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};
