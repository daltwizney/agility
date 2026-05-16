#include "MeshLabSurface.h"

#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInterface.h"

AMeshLabSurface::AMeshLabSurface()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

void AMeshLabSurface::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Regenerate();
}

void AMeshLabSurface::Regenerate()
{
	Mesh->ClearAllMeshSections();

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FLinearColor> Colors;

	switch (Shape)
	{
	case EMeshLabShape::Cube:
		BuildCube(Size, Vertices, Triangles, Normals, UV0, Colors);
		break;
	case EMeshLabShape::Quad:
	default:
		BuildQuad(Size, Vertices, Triangles, Normals, UV0, Colors);
		break;
	}

	const TArray<FProcMeshTangent> Tangents;
	Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, Colors, Tangents, /*bCreateCollision=*/false);

	if (Material)
	{
		Mesh->SetMaterial(0, Material);
	}
}

void AMeshLabSurface::BuildQuad(float Size, TArray<FVector>& Vertices, TArray<int32>& Triangles,
	TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& Colors)
{
	// Square in the XY plane, side length 2 * Size, centered at the actor origin.
	// Winding is CW viewed from +Z (opposite right-hand-rule math) so the normal faces +Z
	// under UE 5.7's culling — empirically established in the Part A triangle test;
	// see "What we learned" in Docs/tutorials/proceduralmesh/00-scope.md.
	const float S = Size;
	Vertices.Add(FVector(-S, -S, 0.0f));  // V0 — bottom-left
	Vertices.Add(FVector( S, -S, 0.0f));  // V1 — bottom-right
	Vertices.Add(FVector( S,  S, 0.0f));  // V2 — top-right
	Vertices.Add(FVector(-S,  S, 0.0f));  // V3 — top-left

	for (int32 i = 0; i < 4; ++i)
	{
		Normals.Add(FVector::UpVector);
	}

	UV0.Add(FVector2D(0.0f, 1.0f));  // BL
	UV0.Add(FVector2D(1.0f, 1.0f));  // BR
	UV0.Add(FVector2D(1.0f, 0.0f));  // TR
	UV0.Add(FVector2D(0.0f, 0.0f));  // TL

	// Classic "smooth gradient" corner palette: BL=black, BR=red, TR=yellow, TL=green.
	// Why this looks seamless where {red, green, blue, black} did not — the color function
	// happens to equal (x, y, 0) at every corner (R encodes X, G encodes Y, B=0 everywhere).
	// Barycentric interp inside each triangle is linear, so each triangle independently
	// recovers the SAME linear function (x, y, 0) — and the two triangles agree across the
	// shared diagonal with no kink in the gradient direction. The seam disappears not because
	// we changed the geometry, but because we picked colors whose bilinear blend is linear.
	Colors.Add(FLinearColor(0.0f, 0.0f, 0.0f));  // BL black
	Colors.Add(FLinearColor(1.0f, 0.0f, 0.0f));  // BR red
	Colors.Add(FLinearColor(1.0f, 1.0f, 0.0f));  // TR yellow
	Colors.Add(FLinearColor(0.0f, 1.0f, 0.0f));  // TL green

	Triangles.Add(0); Triangles.Add(3); Triangles.Add(2);  // BL → TL → TR
	Triangles.Add(0); Triangles.Add(2); Triangles.Add(1);  // BL → TR → BR
}

void AMeshLabSurface::BuildCube(float Size, TArray<FVector>& Vertices, TArray<int32>& Triangles,
	TArray<FVector>& Normals, TArray<FVector2D>& UV0, TArray<FLinearColor>& Colors)
{
	// Cube centered at the actor origin, half-extent = Size in each axis (full side = 2*Size).
	// Each face gets its own 4 verts (24 total) so per-face normals + per-face 0..1 UVs are
	// possible without seams — same trick UE's static-mesh builder uses for hard-edge cubes.
	//
	// Per-face authoring uses the same winding rule the quad established: pick a face origin
	// (BL viewed from outside) and a (Right, Up) basis with Right×Up = +OutwardNormal, then
	// emit verts in the order BL, BR, TR, TL and triangle indices (0,3,2, 0,2,1) (offset by
	// the face's base index). This guarantees the cross-product-opposite-normal invariant and
	// matches the quad's winding so the same logic survives unchanged in BuildQuad above.
	const float S = Size;

	struct FFace
	{
		FVector Origin;   // BL corner viewed from outside
		FVector Right;    // BL → BR
		FVector Up;       // BL → TL
	};
	const FFace Faces[6] =
	{
		// +X
		{ FVector( S, -S, -S), FVector(0,  2*S, 0), FVector(0, 0,  2*S) },
		// -X
		{ FVector(-S,  S, -S), FVector(0, -2*S, 0), FVector(0, 0,  2*S) },
		// +Y
		{ FVector( S,  S, -S), FVector(-2*S, 0, 0), FVector(0, 0,  2*S) },
		// -Y
		{ FVector(-S, -S, -S), FVector( 2*S, 0, 0), FVector(0, 0,  2*S) },
		// +Z
		{ FVector(-S, -S,  S), FVector( 2*S, 0, 0), FVector(0,  2*S, 0) },
		// -Z
		{ FVector(-S,  S, -S), FVector( 2*S, 0, 0), FVector(0, -2*S, 0) },
	};

	for (int32 f = 0; f < 6; ++f)
	{
		const FFace& Face = Faces[f];
		const FVector Normal = Face.Right.Cross(Face.Up).GetSafeNormal();
		const int32 Base = Vertices.Num();

		Vertices.Add(Face.Origin);                                      // BL
		Vertices.Add(Face.Origin + Face.Right);                         // BR
		Vertices.Add(Face.Origin + Face.Right + Face.Up);               // TR
		Vertices.Add(Face.Origin + Face.Up);                            // TL

		for (int32 i = 0; i < 4; ++i) Normals.Add(Normal);

		UV0.Add(FVector2D(0.0f, 1.0f));
		UV0.Add(FVector2D(1.0f, 1.0f));
		UV0.Add(FVector2D(1.0f, 0.0f));
		UV0.Add(FVector2D(0.0f, 0.0f));

		// Liquid / kollie ports key off UV+Time and ignore vertex color; white is a neutral
		// default so any future vertex-color-reading shader still gets a sane signal.
		for (int32 i = 0; i < 4; ++i) Colors.Add(FLinearColor::White);

		Triangles.Add(Base + 0); Triangles.Add(Base + 3); Triangles.Add(Base + 2);
		Triangles.Add(Base + 0); Triangles.Add(Base + 2); Triangles.Add(Base + 1);
	}
}
