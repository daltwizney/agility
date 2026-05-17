#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FlappyBirdHUD.generated.h"

// Procedural HUD that mirrors the kollie Compose layout: yellow score top-center,
// pink "BEST n" pill under it, and a centered banner (cyan "TAP TO FLAP" while
// Ready, red "GAME OVER" while Dead). Everything is drawn via DrawHUD primitives
// — no UMG widget assets required, keeping the HUD authoring entirely in C++.
UCLASS()
class AGILITY_API AFlappyBirdHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	// Visual tunables.
	UPROPERTY(EditAnywhere, Category = "HUD") float ScoreTextScale     = 6.0f;
	UPROPERTY(EditAnywhere, Category = "HUD") float ScoreTextTopMargin = 30.0f;
	UPROPERTY(EditAnywhere, Category = "HUD") float BestTextScale      = 1.2f;
	UPROPERTY(EditAnywhere, Category = "HUD") float BannerTitleScale   = 2.8f;
	UPROPERTY(EditAnywhere, Category = "HUD") float BannerSubtitleScale= 1.0f;
};
