#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FlappyBirdHUD.generated.h"

UCLASS()
class AGILITY_API AFlappyBirdHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "HUD")
	float ScoreTextScale = 4.0f;

	UPROPERTY(EditAnywhere, Category = "HUD")
	float ScoreTextTopMarginPx = 36.0f;
};
