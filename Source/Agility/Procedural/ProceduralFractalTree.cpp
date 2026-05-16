#include "ProceduralFractalTree.h"

#include "Math/RandomStream.h"
#include "ProceduralMeshComponent.h"

AProceduralFractalTree::AProceduralFractalTree()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

void AProceduralFractalTree::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Regenerate();
}

static FVector PickPerpendicular(const FVector& Dir)
{
	const FVector Helper = (FMath::Abs(Dir.Z) < 0.99f) ? FVector::UpVector : FVector::ForwardVector;
	return FVector::CrossProduct(Dir, Helper).GetSafeNormal();
}

void AProceduralFractalTree::EmitCylinder(const FVector& Start, const FVector& End, float StartRadius, float EndRadius, int32 Sides, const FLinearColor& BaseColor,
	TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& Colors) const
{
	const FVector Axis = (End - Start).GetSafeNormal();
	if (Axis.IsNearlyZero() || Sides < 3)
	{
		return;
	}

	const FVector Tangent = PickPerpendicular(Axis);
	const FVector Bitangent = FVector::CrossProduct(Axis, Tangent).GetSafeNormal();

	const int32 BaseIndex = Vertices.Num();

	for (int32 i = 0; i < Sides; ++i)
	{
		const float Angle = (2.0f * PI * i) / Sides;
		const FVector Radial = FMath::Cos(Angle) * Tangent + FMath::Sin(Angle) * Bitangent;

		Vertices.Add(Start + Radial * StartRadius);
		Vertices.Add(End + Radial * EndRadius);
		Normals.Add(Radial);
		Normals.Add(Radial);
		UV0.Add(FVector2D(static_cast<float>(i) / Sides, 0.0f));
		UV0.Add(FVector2D(static_cast<float>(i) / Sides, 1.0f));
		Colors.Add(BaseColor);
		Colors.Add(BaseColor);
	}

	for (int32 i = 0; i < Sides; ++i)
	{
		const int32 i0 = BaseIndex + i * 2;
		const int32 i1 = BaseIndex + i * 2 + 1;
		const int32 i2 = BaseIndex + ((i + 1) % Sides) * 2;
		const int32 i3 = BaseIndex + ((i + 1) % Sides) * 2 + 1;

		Triangles.Add(i0);
		Triangles.Add(i1);
		Triangles.Add(i2);

		Triangles.Add(i2);
		Triangles.Add(i1);
		Triangles.Add(i3);
	}
}

void AProceduralFractalTree::Branch(const FVector& Start, const FVector& Direction, float Length, float Radius, int32 Depth, FRandomStream& Rng,
	TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& Colors) const
{
	if (Depth >= MaxDepth || Length < 1.0f || Radius < 0.1f)
	{
		return;
	}

	const FVector ParentDir = Direction.GetSafeNormal();
	const FVector End = Start + ParentDir * Length;
	const float EndRadius = Radius * RadiusDecay;

	// Color shifts trunk → leaf tip across the recursion so the tree reads as a tree even with WorldGridMaterial.
	const float DepthRatio = (MaxDepth > 1) ? (static_cast<float>(Depth) / (MaxDepth - 1)) : 0.0f;
	const FLinearColor TrunkColor(0.30f, 0.18f, 0.08f);
	const FLinearColor LeafColor(0.20f, 0.55f, 0.15f);
	const FLinearColor SegmentColor = FMath::Lerp(TrunkColor, LeafColor, DepthRatio);

	EmitCylinder(Start, End, Radius, EndRadius, SidesPerBranch, SegmentColor, Vertices, Triangles, Normals, UV0, Colors);

	const FVector ParentTangent = PickPerpendicular(ParentDir);
	const FVector ParentBitangent = FVector::CrossProduct(ParentDir, ParentTangent).GetSafeNormal();

	for (int32 i = 0; i < BranchesPerNode; ++i)
	{
		const float Azimuth = (2.0f * PI * i) / BranchesPerNode + Rng.FRandRange(-0.2f, 0.2f);
		const float Pitch = FMath::DegreesToRadians(BranchAngle + Rng.FRandRange(-AngleJitter, AngleJitter));

		const FVector OffAxis = FMath::Cos(Azimuth) * ParentTangent + FMath::Sin(Azimuth) * ParentBitangent;
		const FVector NewDir = (FMath::Cos(Pitch) * ParentDir + FMath::Sin(Pitch) * OffAxis).GetSafeNormal();

		Branch(End, NewDir, Length * LengthDecay, EndRadius, Depth + 1, Rng, Vertices, Triangles, Normals, UV0, Colors);
	}
}

void AProceduralFractalTree::Regenerate()
{
	if (!Mesh)
	{
		return;
	}

	Mesh->ClearAllMeshSections();

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FLinearColor> Colors;
	const TArray<FProcMeshTangent> Tangents;

	FRandomStream Rng(Seed);
	Branch(FVector::ZeroVector, FVector::UpVector, TrunkLength, TrunkRadius, 0, Rng, Vertices, Triangles, Normals, UV0, Colors);

	if (Vertices.Num() == 0 || Triangles.Num() == 0)
	{
		return;
	}

	Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, Colors, Tangents, /*bCreateCollision=*/false);
}
