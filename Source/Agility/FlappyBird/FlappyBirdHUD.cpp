#include "FlappyBirdHUD.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "FlappyBirdGameMode.h"

namespace
{
	// Palette taken from kollie's FlappyBirdHUD.kt Color literals.
	const FLinearColor ScoreYellow(1.0f, 0.878f, 0.40f, 1.0f);  // 0xFFFFE066
	const FLinearColor BestPink(1.0f, 0.482f, 0.847f, 1.0f);    // 0xFFFF7BD8
	const FLinearColor BannerCyan(0.40f, 0.882f, 1.0f, 1.0f);   // 0xFF66E1FF
	const FLinearColor BannerRed(1.0f, 0.302f, 0.478f, 1.0f);   // 0xFFFF4D7A
	const FLinearColor BannerSubtitle(1.0f, 1.0f, 1.0f, 0.85f);
	const FLinearColor PillBg(0.0f, 0.0f, 0.0f, 0.35f);
	const FLinearColor BannerBg(0.0f, 0.0f, 0.0f, 0.55f);

	void GetSize(UCanvas* Canvas, const FString& Text, UFont* Font, float Scale, float& OutW, float& OutH)
	{
		OutW = 0.0f; OutH = 0.0f;
		Canvas->StrLen(Font, Text, OutW, OutH);
		OutW *= Scale; OutH *= Scale;
	}

	void DrawTextScaled(UCanvas* Canvas, const FString& Text, UFont* Font, float Scale, float X, float Y, const FLinearColor& Color)
	{
		FCanvasTextItem Item(FVector2D(X, Y), FText::FromString(Text), Font, Color);
		Item.Scale = FVector2D(Scale, Scale);
		Item.EnableShadow(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));
		Canvas->DrawItem(Item);
	}

	void DrawFilledRect(UCanvas* Canvas, float X, float Y, float W, float H, const FLinearColor& Color)
	{
		FCanvasTileItem Tile(FVector2D(X, Y), FVector2D(W, H), Color);
		Tile.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(Tile);
	}
}

void AFlappyBirdHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas || !GEngine)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const AFlappyBirdGameMode* GM = World ? World->GetAuthGameMode<AFlappyBirdGameMode>() : nullptr;
	if (!GM)
	{
		return;
	}

	UFont* Font = GEngine->GetLargeFont();
	if (!Font)
	{
		return;
	}

	const float ScreenW = Canvas->ClipX;
	const float ScreenH = Canvas->ClipY;

	// --- Score (top center, yellow)
	const FString ScoreStr = FString::FromInt(GM->GetScore());
	float ScoreW = 0.0f, ScoreH = 0.0f;
	GetSize(Canvas, ScoreStr, Font, ScoreTextScale, ScoreW, ScoreH);
	const float ScoreX = (ScreenW - ScoreW) * 0.5f;
	const float ScoreY = ScoreTextTopMargin;
	DrawTextScaled(Canvas, ScoreStr, Font, ScoreTextScale, ScoreX, ScoreY, ScoreYellow);

	// --- BEST n pill (under score, pink text on translucent black)
	const FString BestStr = FString::Printf(TEXT("BEST %d"), GM->GetBest());
	float BestW = 0.0f, BestH = 0.0f;
	GetSize(Canvas, BestStr, Font, BestTextScale, BestW, BestH);
	const float PillPadX = 14.0f;
	const float PillPadY = 6.0f;
	const float PillW = BestW + PillPadX * 2.0f;
	const float PillH = BestH + PillPadY * 2.0f;
	const float PillX = (ScreenW - PillW) * 0.5f;
	const float PillY = ScoreY + ScoreH + 12.0f;
	DrawFilledRect(Canvas, PillX, PillY, PillW, PillH, PillBg);
	DrawTextScaled(Canvas, BestStr, Font, BestTextScale, PillX + PillPadX, PillY + PillPadY, BestPink);

	// --- Phase banner (center screen, Ready / Dead only)
	const EFlappyPhase Phase = GM->GetPhase();
	if (Phase == EFlappyPhase::Playing)
	{
		return;
	}

	const FString Title = (Phase == EFlappyPhase::Ready) ? TEXT("TAP TO FLAP") : TEXT("GAME OVER");
	const FString Sub   = (Phase == EFlappyPhase::Ready) ? TEXT("fly through the gaps") : TEXT("tap to retry");
	const FLinearColor TitleColor = (Phase == EFlappyPhase::Ready) ? BannerCyan : BannerRed;

	float TitleW = 0.0f, TitleH = 0.0f;
	float SubW = 0.0f, SubH = 0.0f;
	GetSize(Canvas, Title, Font, BannerTitleScale, TitleW, TitleH);
	GetSize(Canvas, Sub,   Font, BannerSubtitleScale, SubW, SubH);
	const float BannerPadX = 28.0f;
	const float BannerPadY = 18.0f;
	const float InnerW = FMath::Max(TitleW, SubW);
	const float InnerH = TitleH + 8.0f + SubH;
	const float BannerW = InnerW + BannerPadX * 2.0f;
	const float BannerH = InnerH + BannerPadY * 2.0f;
	const float BannerX = (ScreenW - BannerW) * 0.5f;
	const float BannerY = (ScreenH - BannerH) * 0.5f;
	DrawFilledRect(Canvas, BannerX, BannerY, BannerW, BannerH, BannerBg);

	const float TitleX = BannerX + (BannerW - TitleW) * 0.5f;
	const float TitleY = BannerY + BannerPadY;
	DrawTextScaled(Canvas, Title, Font, BannerTitleScale, TitleX, TitleY, TitleColor);

	const float SubX = BannerX + (BannerW - SubW) * 0.5f;
	const float SubY = TitleY + TitleH + 8.0f;
	DrawTextScaled(Canvas, Sub, Font, BannerSubtitleScale, SubX, SubY, BannerSubtitle);
}
