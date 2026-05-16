#include "PipePair.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "FlappyBird.h"
#include "FlappyBirdGameMode.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogFlappyPipe, Log, All);

namespace
{
	// Tube band between two rings on the Z axis. CenterXY lets the caller offset the whole tube in X/Y;
	// in this project that stays at the origin since each pipe-half is its own actor component.
	void EmitTubeBand(
		float ZA, float RadiusA,
		float ZB, float RadiusB,
		const FVector2D& CenterXY,
		int32 Sides,
		const FLinearColor& Color,
		TArray<FVector>& Vertices,
		TArray<int32>& Triangles,
		TArray<FVector>& Normals,
		TArray<FVector2D>& UV0,
		TArray<FLinearColor>& Colors)
	{
		Sides = FMath::Max(Sides, 3);
		const int32 BaseIndex = Vertices.Num();

		// Side-surface normal points outward (away from axis). For a frustum, the normal also tilts along Z
		// proportional to (RadiusA - RadiusB) / (ZB - ZA), but for our straight cylinders these are equal-radius
		// so the normal is purely radial. Keep the general form anyway.
		const float DZ = ZB - ZA;
		const float DR = RadiusB - RadiusA;
		const float SlopeLen = FMath::Sqrt(DZ * DZ + DR * DR);

		for (int32 i = 0; i < Sides; ++i)
		{
			const float Angle = (2.0f * PI * i) / Sides;
			const float CosA = FMath::Cos(Angle);
			const float SinA = FMath::Sin(Angle);
			const FVector Radial(CosA, SinA, 0.0f);

			FVector Normal = Radial;
			if (SlopeLen > KINDA_SMALL_NUMBER)
			{
				// Tilt the radial normal by the slope so frustums shade correctly.
				Normal = (Radial * DZ - FVector(0.0f, 0.0f, DR)).GetSafeNormal();
				if (FVector::DotProduct(Normal, Radial) < 0.0f)
				{
					Normal = -Normal;
				}
			}

			const FVector VA(CenterXY.X + CosA * RadiusA, CenterXY.Y + SinA * RadiusA, ZA);
			const FVector VB(CenterXY.X + CosA * RadiusB, CenterXY.Y + SinA * RadiusB, ZB);

			Vertices.Add(VA);
			Vertices.Add(VB);
			Normals.Add(Normal);
			Normals.Add(Normal);
			UV0.Add(FVector2D(static_cast<float>(i) / Sides, 0.0f));
			UV0.Add(FVector2D(static_cast<float>(i) / Sides, 1.0f));
			Colors.Add(Color);
			Colors.Add(Color);
		}

		for (int32 i = 0; i < Sides; ++i)
		{
			const int32 I0 = BaseIndex + i * 2;
			const int32 I1 = BaseIndex + i * 2 + 1;
			const int32 I2 = BaseIndex + ((i + 1) % Sides) * 2;
			const int32 I3 = BaseIndex + ((i + 1) % Sides) * 2 + 1;

			Triangles.Add(I0);
			Triangles.Add(I1);
			Triangles.Add(I3);

			Triangles.Add(I0);
			Triangles.Add(I3);
			Triangles.Add(I2);
		}

		// Inside-facing duplicates: same vertex positions but inward normals + reversed winding, so the
		// cylinder is visible from inside too. Without this, looking through the nearly-edge-on rim cap
		// shows straight through the pipe (back-face culling hides the inside walls and far back arc).
		const int32 InsideBase = Vertices.Num();
		for (int32 i = 0; i < Sides; ++i)
		{
			const float Angle = (2.0f * PI * i) / Sides;
			const float CosA = FMath::Cos(Angle);
			const float SinA = FMath::Sin(Angle);
			const FVector Radial(CosA, SinA, 0.0f);

			FVector OutwardNormal = Radial;
			if (SlopeLen > KINDA_SMALL_NUMBER)
			{
				OutwardNormal = (Radial * DZ - FVector(0.0f, 0.0f, DR)).GetSafeNormal();
				if (FVector::DotProduct(OutwardNormal, Radial) < 0.0f)
				{
					OutwardNormal = -OutwardNormal;
				}
			}
			const FVector InwardNormal = -OutwardNormal;

			const FVector VA(CenterXY.X + CosA * RadiusA, CenterXY.Y + SinA * RadiusA, ZA);
			const FVector VB(CenterXY.X + CosA * RadiusB, CenterXY.Y + SinA * RadiusB, ZB);

			Vertices.Add(VA);
			Vertices.Add(VB);
			Normals.Add(InwardNormal);
			Normals.Add(InwardNormal);
			UV0.Add(FVector2D(static_cast<float>(i) / Sides, 0.0f));
			UV0.Add(FVector2D(static_cast<float>(i) / Sides, 1.0f));
			Colors.Add(Color);
			Colors.Add(Color);
		}

		for (int32 i = 0; i < Sides; ++i)
		{
			const int32 I0 = InsideBase + i * 2;
			const int32 I1 = InsideBase + i * 2 + 1;
			const int32 I2 = InsideBase + ((i + 1) % Sides) * 2;
			const int32 I3 = InsideBase + ((i + 1) % Sides) * 2 + 1;

			Triangles.Add(I0);
			Triangles.Add(I3);
			Triangles.Add(I1);

			Triangles.Add(I0);
			Triangles.Add(I2);
			Triangles.Add(I3);
		}
	}

	// Flat annulus at a constant Z. InnerRadius=0 makes it a solid disc (end cap).
	// NormalZ is +1 or -1 — which way the visible face points.
	void EmitFlatRing(
		float Z,
		float InnerRadius,
		float OuterRadius,
		float NormalZ,
		const FVector2D& CenterXY,
		int32 Sides,
		const FLinearColor& Color,
		TArray<FVector>& Vertices,
		TArray<int32>& Triangles,
		TArray<FVector>& Normals,
		TArray<FVector2D>& UV0,
		TArray<FLinearColor>& Colors)
	{
		Sides = FMath::Max(Sides, 3);
		const FVector Normal(0.0f, 0.0f, NormalZ);

		if (InnerRadius <= KINDA_SMALL_NUMBER)
		{
			// Solid disc: center vertex + ring fan.
			const int32 CenterIndex = Vertices.Num();
			Vertices.Add(FVector(CenterXY.X, CenterXY.Y, Z));
			Normals.Add(Normal);
			UV0.Add(FVector2D(0.5f, 0.5f));
			Colors.Add(Color);

			const int32 RingStart = Vertices.Num();
			for (int32 i = 0; i < Sides; ++i)
			{
				const float Angle = (2.0f * PI * i) / Sides;
				const float CosA = FMath::Cos(Angle);
				const float SinA = FMath::Sin(Angle);
				Vertices.Add(FVector(CenterXY.X + CosA * OuterRadius, CenterXY.Y + SinA * OuterRadius, Z));
				Normals.Add(Normal);
				UV0.Add(FVector2D(0.5f + 0.5f * CosA, 0.5f + 0.5f * SinA));
				Colors.Add(Color);
			}

			for (int32 i = 0; i < Sides; ++i)
			{
				const int32 Next = (i + 1) % Sides;
				Triangles.Add(CenterIndex);
				// Winding flipped based on NormalZ so the visible side faces the right way.
				if (NormalZ >= 0.0f)
				{
					Triangles.Add(RingStart + i);
					Triangles.Add(RingStart + Next);
				}
				else
				{
					Triangles.Add(RingStart + Next);
					Triangles.Add(RingStart + i);
				}
			}
		}
		else
		{
			// Annulus: two rings + quad strip.
			const int32 BaseIndex = Vertices.Num();
			for (int32 i = 0; i < Sides; ++i)
			{
				const float Angle = (2.0f * PI * i) / Sides;
				const float CosA = FMath::Cos(Angle);
				const float SinA = FMath::Sin(Angle);
				Vertices.Add(FVector(CenterXY.X + CosA * InnerRadius, CenterXY.Y + SinA * InnerRadius, Z));
				Vertices.Add(FVector(CenterXY.X + CosA * OuterRadius, CenterXY.Y + SinA * OuterRadius, Z));
				Normals.Add(Normal);
				Normals.Add(Normal);
				const float U = static_cast<float>(i) / Sides;
				UV0.Add(FVector2D(U, 0.0f));
				UV0.Add(FVector2D(U, 1.0f));
				Colors.Add(Color);
				Colors.Add(Color);
			}

			for (int32 i = 0; i < Sides; ++i)
			{
				const int32 Next = (i + 1) % Sides;
				const int32 IInnerA = BaseIndex + i * 2;
				const int32 IOuterA = BaseIndex + i * 2 + 1;
				const int32 IInnerB = BaseIndex + Next * 2;
				const int32 IOuterB = BaseIndex + Next * 2 + 1;

				if (NormalZ >= 0.0f)
				{
					Triangles.Add(IInnerA);
					Triangles.Add(IOuterA);
					Triangles.Add(IOuterB);

					Triangles.Add(IInnerA);
					Triangles.Add(IOuterB);
					Triangles.Add(IInnerB);
				}
				else
				{
					Triangles.Add(IInnerA);
					Triangles.Add(IOuterB);
					Triangles.Add(IOuterA);

					Triangles.Add(IInnerA);
					Triangles.Add(IInnerB);
					Triangles.Add(IOuterB);
				}
			}
		}
	}
}

APipePair::APipePair()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BottomPipeMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("BottomPipeMesh"));
	BottomPipeMesh->SetupAttachment(Root);

	TopPipeMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TopPipeMesh"));
	TopPipeMesh->SetupAttachment(Root);

	auto MakeOverlapBox = [&](FName Name) -> UBoxComponent*
	{
		UBoxComponent* Box = CreateDefaultSubobject<UBoxComponent>(Name);
		Box->SetupAttachment(Root);
		Box->SetCollisionProfileName(TEXT("OverlapAll"));
		Box->SetGenerateOverlapEvents(true);
		return Box;
	};

	BottomHitBox = MakeOverlapBox(TEXT("BottomHitBox"));
	TopHitBox = MakeOverlapBox(TEXT("TopHitBox"));
	ScoreTrigger = MakeOverlapBox(TEXT("ScoreTrigger"));

	BottomHitBox->OnComponentBeginOverlap.AddDynamic(this, &APipePair::OnHitBoxOverlap);
	TopHitBox->OnComponentBeginOverlap.AddDynamic(this, &APipePair::OnHitBoxOverlap);
	ScoreTrigger->OnComponentBeginOverlap.AddDynamic(this, &APipePair::OnScoreTriggerOverlap);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicShapeMaterial(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BasicShapeMaterial.Succeeded())
	{
		ColoredParentMaterial = BasicShapeMaterial.Object;
	}
}

void APipePair::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildPipes();
}

void APipePair::BuildPipes()
{
	if (!BottomPipeMesh || !TopPipeMesh)
	{
		return;
	}

	BottomPipeMesh->ClearAllMeshSections();
	TopPipeMesh->ClearAllMeshSections();

	const float GapBottomZ = GapCenterZ - GapHeight * 0.5f;
	const float GapTopZ = GapCenterZ + GapHeight * 0.5f;

	auto BuildHalf = [&](UProceduralMeshComponent* Mesh, float GapEndZ, float DirZ)
	{
		// Z marker positions along the pipe, from gap end (Z0) outward.
		const float Z0 = GapEndZ;                          // rim top (open face)
		const float Z1 = GapEndZ + DirZ * RimHeight;       // rim base / start of main body
		const float Z3 = GapEndZ + DirZ * PipeExtent;      // far end of main body
		const FVector2D Center(0.0f, 0.0f);

		// Section 0 — main pipe body (cylinder + far-end cap).
		{
			TArray<FVector> V; TArray<int32> T; TArray<FVector> N; TArray<FVector2D> U; TArray<FLinearColor> C;
			EmitTubeBand(Z1, PipeRadius, Z3, PipeRadius, Center, Sides, PipeColor, V, T, N, U, C);
			EmitFlatRing(Z3, 0.0f, PipeRadius, DirZ, Center, Sides, PipeColor, V, T, N, U, C);
			const TArray<FProcMeshTangent> Tangents;
			Mesh->CreateMeshSection_LinearColor(0, V, T, N, U, C, Tangents, false);
			ApplySectionColor(Mesh, 0, PipeColor);
		}

		// Section 1 — rim flange (wider band + shoulder annulus + open-face cap).
		// Separate section so we can tint it slightly darker than the main body.
		{
			TArray<FVector> V; TArray<int32> T; TArray<FVector> N; TArray<FVector2D> U; TArray<FLinearColor> C;
			EmitTubeBand(Z0, RimRadius, Z1, RimRadius, Center, Sides, RimColor, V, T, N, U, C);
			// Shoulder annulus on the underside of the flange — facing away from the gap.
			EmitFlatRing(Z1, PipeRadius, RimRadius, DirZ, Center, Sides, RimColor, V, T, N, U, C);
			// Disc that caps the open (gap-facing) end — facing toward the gap.
			EmitFlatRing(Z0, 0.0f, RimRadius, -DirZ, Center, Sides, RimColor, V, T, N, U, C);
			const TArray<FProcMeshTangent> Tangents;
			Mesh->CreateMeshSection_LinearColor(1, V, T, N, U, C, Tangents, false);
			ApplySectionColor(Mesh, 1, RimColor);
		}
	};

	BuildHalf(BottomPipeMesh, GapBottomZ, -1.0f);
	BuildHalf(TopPipeMesh, GapTopZ, +1.0f);

	// Collision volumes. Use main PipeRadius (not RimRadius) for the box extent — slightly forgiving by
	// design, since the rim flange sticks out a bit further than the hit box.
	if (BottomHitBox)
	{
		BottomHitBox->SetRelativeLocation(FVector(0.0f, 0.0f, GapBottomZ - PipeExtent * 0.5f));
		BottomHitBox->SetBoxExtent(FVector(PipeRadius, PipeRadius, PipeExtent * 0.5f), /*bUpdateOverlaps=*/true);
	}
	if (TopHitBox)
	{
		TopHitBox->SetRelativeLocation(FVector(0.0f, 0.0f, GapTopZ + PipeExtent * 0.5f));
		TopHitBox->SetBoxExtent(FVector(PipeRadius, PipeRadius, PipeExtent * 0.5f), true);
	}
	if (ScoreTrigger)
	{
		ScoreTrigger->SetRelativeLocation(FVector(0.0f, 0.0f, GapCenterZ));
		// Thin in X (just a "score line") but full gap height in Z and wide in Y for forgiving overlap detection.
		ScoreTrigger->SetBoxExtent(FVector(15.0f, PipeRadius, GapHeight * 0.5f), true);
	}
}

void APipePair::ApplySectionColor(UProceduralMeshComponent* Mesh, int32 SectionIndex, const FLinearColor& Color)
{
	if (!Mesh || !ColoredParentMaterial)
	{
		return;
	}
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(ColoredParentMaterial, Mesh);
	if (!MID)
	{
		return;
	}
	MID->SetVectorParameterValue(TEXT("Color"), Color);
	Mesh->SetMaterial(SectionIndex, MID);
}

void APipePair::OnHitBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<AFlappyBird>(OtherActor))
	{
		return;
	}
	// Part 4 wires this into a real game-over flow. For now, just leave a breadcrumb.
	UE_LOG(LogFlappyPipe, Display, TEXT("Bird hit pipe (%s)"), *GetName());
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
			GM->AddScore(1);
			bScored = true;
		}
	}
}

void APipePair::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector Location = GetActorLocation();
	Location.X -= ScrollSpeed * DeltaSeconds;
	SetActorLocation(Location);

	if (Location.X < DespawnX)
	{
		Destroy();
	}
}
