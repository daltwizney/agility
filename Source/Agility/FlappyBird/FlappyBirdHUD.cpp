#include "FlappyBirdHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "FlappyBirdGameMode.h"

void AFlappyBirdHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas || !GEngine)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const AFlappyBirdGameMode* GM = World->GetAuthGameMode<AFlappyBirdGameMode>();
	if (!GM)
	{
		return;
	}

	UFont* Font = GEngine->GetLargeFont();
	const FString Text = FString::Printf(TEXT("%d"), GM->GetScore());

	float TextWidth = 0.0f;
	float TextHeight = 0.0f;
	GetTextSize(Text, TextWidth, TextHeight, Font, ScoreTextScale);

	const float X = (Canvas->ClipX - TextWidth) * 0.5f;
	const float Y = ScoreTextTopMarginPx;

	// Cheap drop-shadow so the white score reads against bright sky / cloud backgrounds.
	DrawText(Text, FLinearColor(0.0f, 0.0f, 0.0f, 0.6f), X + 3.0f, Y + 3.0f, Font, ScoreTextScale);
	DrawText(Text, FLinearColor::White, X, Y, Font, ScoreTextScale);
}
