#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FlappyBirdGameMode.generated.h"

UCLASS()
class AGILITY_API AFlappyBirdGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFlappyBirdGameMode();

	void AddScore(int32 Amount = 1);
	int32 GetScore() const { return Score; }

private:
	int32 Score = 0;
};
