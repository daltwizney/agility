#include "PipePair.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "FlappyBird.h"
#include "FlappyBirdConstants.h"
#include "FlappyBirdGameMode.h"
#include "FlappyNeonMaterials.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogFlappyPipe, Log, All);

using namespace FlappyBirdConstants;

APipePair::APipePair()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	auto MakeMesh = [&](FName Name) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Mesh->SetupAttachment(Root);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCastShadow(false);
		return Mesh;
	};
	TopShaftMesh    = MakeMesh(TEXT("TopShaftMesh"));
	TopCapMesh      = MakeMesh(TEXT("TopCapMesh"));
	BottomShaftMesh = MakeMesh(TEXT("BottomShaftMesh"));
	BottomCapMesh   = MakeMesh(TEXT("BottomCapMesh"));

	auto MakeBox = [&](FName Name) -> UBoxComponent*
	{
		UBoxComponent* Box = CreateDefaultSubobject<UBoxComponent>(Name);
		Box->SetupAttachment(Root);
		Box->SetCollisionProfileName(TEXT("OverlapAll"));
		Box->SetGenerateOverlapEvents(true);
		return Box;
	};
	TopHitBox    = MakeBox(TEXT("TopHitBox"));
	BottomHitBox = MakeBox(TEXT("BottomHitBox"));
	ScoreTrigger = MakeBox(TEXT("ScoreTrigger"));

	TopHitBox->OnComponentBeginOverlap.AddDynamic(this, &APipePair::OnHitBoxOverlap);
	BottomHitBox->OnComponentBeginOverlap.AddDynamic(this, &APipePair::OnHitBoxOverlap);
	ScoreTrigger->OnComponentBeginOverlap.AddDynamic(this, &APipePair::OnScoreTriggerOverlap);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		TopShaftMesh->SetStaticMesh(CubeMesh.Object);
		TopCapMesh->SetStaticMesh(CubeMesh.Object);
		BottomShaftMesh->SetStaticMesh(CubeMesh.Object);
		BottomCapMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void APipePair::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildPipes();
}

void APipePair::BuildPipes()
{
	const float GapBottomZ = GapCenterZ - PipeGapHeight * 0.5f;
	const float GapTopZ    = GapCenterZ + PipeGapHeight * 0.5f;

	const float HalfW = PipeColumnHalfWidth;
	const float HalfD = PipeColumnHalfDepth;
	const float CapHalfH = PipeCapHalfHeight;
	const float CapW     = HalfW * 2.0f * PipeCapWidthMul;   // wider than shaft
	const float CapD     = HalfD * 2.0f * PipeCapWidthMul;

	// /Engine/BasicShapes/Cube is a 100×100×100 cm centered cube — component
	// scale therefore equals the desired side length in metres ×100. We pass
	// the side length in cm directly by dividing by 100 in scale, but the math
	// is simpler if we scale by `SideLengthCm / 100`. Use a tiny helper.
	auto ToCubeScale = [](float SideX, float SideY, float SideZ)
	{
		return FVector(SideX / 100.0f, SideY / 100.0f, SideZ / 100.0f);
	};

	const FlappyNeonPalette::FNeonSpec& ShaftSpec = bUseCyan ? FlappyNeonPalette::PipeShaftCyan : FlappyNeonPalette::PipeShaftMagenta;
	const FlappyNeonPalette::FNeonSpec& CapSpec   = bUseCyan ? FlappyNeonPalette::PipeCapCyan   : FlappyNeonPalette::PipeCapMagenta;

	// --- Top column: from GapTopZ up to PipeColumnTopZ.
	{
		const float TopBot = GapTopZ;
		const float TopTop = PipeColumnTopZ;
		const float Height = TopTop - TopBot;
		const float CenterZ = (TopBot + TopTop) * 0.5f;

		if (TopShaftMesh)
		{
			TopShaftMesh->SetRelativeLocation(FVector(0.0f, 0.0f, CenterZ));
			TopShaftMesh->SetRelativeScale3D(ToCubeScale(HalfW * 2.0f, HalfD * 2.0f, Height));
			if (UMaterialInstanceDynamic* MID = FlappyNeonMaterials::MakeMID(TopShaftMesh, ShaftSpec))
			{
				TopShaftMesh->SetMaterial(0, MID);
			}
		}
		// Cap sits at the gap-facing (bottom) end of the top column.
		if (TopCapMesh)
		{
			const float CapZ = CenterZ - Height * 0.5f + CapHalfH;
			TopCapMesh->SetRelativeLocation(FVector(0.0f, 0.0f, CapZ));
			TopCapMesh->SetRelativeScale3D(ToCubeScale(CapW, CapD, CapHalfH * 2.0f));
			if (UMaterialInstanceDynamic* MID = FlappyNeonMaterials::MakeMID(TopCapMesh, CapSpec))
			{
				TopCapMesh->SetMaterial(0, MID);
			}
		}
		if (TopHitBox)
		{
			TopHitBox->SetRelativeLocation(FVector(0.0f, 0.0f, CenterZ));
			TopHitBox->SetBoxExtent(FVector(HalfW, HalfD, Height * 0.5f), /*bUpdateOverlaps=*/true);
		}
	}

	// --- Bottom column: from GapBottomZ down to PipeColumnBottomZ.
	{
		const float BotTop = GapBottomZ;
		const float BotBot = PipeColumnBottomZ;
		const float Height = BotTop - BotBot;            // positive
		const float CenterZ = (BotTop + BotBot) * 0.5f;

		if (BottomShaftMesh)
		{
			BottomShaftMesh->SetRelativeLocation(FVector(0.0f, 0.0f, CenterZ));
			BottomShaftMesh->SetRelativeScale3D(ToCubeScale(HalfW * 2.0f, HalfD * 2.0f, Height));
			if (UMaterialInstanceDynamic* MID = FlappyNeonMaterials::MakeMID(BottomShaftMesh, ShaftSpec))
			{
				BottomShaftMesh->SetMaterial(0, MID);
			}
		}
		// Cap sits at the gap-facing (top) end of the bottom column.
		if (BottomCapMesh)
		{
			const float CapZ = CenterZ + Height * 0.5f - CapHalfH;
			BottomCapMesh->SetRelativeLocation(FVector(0.0f, 0.0f, CapZ));
			BottomCapMesh->SetRelativeScale3D(ToCubeScale(CapW, CapD, CapHalfH * 2.0f));
			if (UMaterialInstanceDynamic* MID = FlappyNeonMaterials::MakeMID(BottomCapMesh, CapSpec))
			{
				BottomCapMesh->SetMaterial(0, MID);
			}
		}
		if (BottomHitBox)
		{
			BottomHitBox->SetRelativeLocation(FVector(0.0f, 0.0f, CenterZ));
			BottomHitBox->SetBoxExtent(FVector(HalfW, HalfD, Height * 0.5f), true);
		}
	}

	// --- Score trigger: thin in X (a "line"), full gap height in Z, ~90% of column depth in Y.
	if (ScoreTrigger)
	{
		ScoreTrigger->SetRelativeLocation(FVector(0.0f, 0.0f, GapCenterZ));
		ScoreTrigger->SetBoxExtent(FVector(12.0f, HalfD * 0.9f, PipeGapHeight * 0.5f), true);
	}
}

void APipePair::OnHitBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<AFlappyBird>(OtherActor))
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (AFlappyBirdGameMode* GM = World->GetAuthGameMode<AFlappyBirdGameMode>())
		{
			GM->NotifyBirdHit(this);
		}
	}
}

void APipePair::OnScoreTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bScored || !Cast<AFlappyBird>(OtherActor))
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (AFlappyBirdGameMode* GM = World->GetAuthGameMode<AFlappyBirdGameMode>())
		{
			GM->NotifyBirdScored();
			bScored = true;
		}
	}
}

void APipePair::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Only scroll while the game is in PLAYING phase. The kollie controller
	// gates the same way — keeps the bird's dead-fall animation uncluttered.
	UWorld* World = GetWorld();
	const AFlappyBirdGameMode* GM = World ? World->GetAuthGameMode<AFlappyBirdGameMode>() : nullptr;
	if (GM && !GM->IsPlaying())
	{
		return;
	}

	FVector L = GetActorLocation();
	L.X -= PipeScrollSpeed * DeltaSeconds;
	SetActorLocation(L);

	if (L.X < PipeDespawnX)
	{
		Destroy();
	}
}
