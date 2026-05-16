#include "FlappyBirdGameMode.h"

#include "FlappyBird.h"
#include "FlappyBirdHUD.h"

AFlappyBirdGameMode::AFlappyBirdGameMode()
{
	DefaultPawnClass = AFlappyBird::StaticClass();
	HUDClass = AFlappyBirdHUD::StaticClass();
}

void AFlappyBirdGameMode::AddScore(int32 Amount)
{
	Score += Amount;
}
