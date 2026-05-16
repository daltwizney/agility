#include "MeshLabScene.h"

#include "MeshLabSurface.h"
#include "ProceduralMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AMeshLabScene::AMeshLabScene()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CubeMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CubeMesh"));
	CubeMesh->SetupAttachment(SceneRoot);

	CylinderWitness = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CylinderWitness"));
	CylinderWitness->SetupAttachment(SceneRoot);

	ConeWitness = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ConeWitness"));
	ConeWitness->SetupAttachment(SceneRoot);

	SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
	SunLight->SetupAttachment(SceneRoot);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(SceneRoot);

	// CubeMaterial is intentionally left unset by default — the Agility plugin ships no
	// content, so we don't hardcode a /Game/ path here. Assign the material in the editor's
	// Details panel on each placed AMeshLabScene instance.

	// Engine basic-shape static meshes — they ship with default lit materials assigned to
	// their slots, so dropping them in unmodified gives us a lighting witness for free.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		CylinderWitness->SetStaticMesh(CylinderMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(
		TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (ConeMesh.Succeeded())
	{
		ConeWitness->SetStaticMesh(ConeMesh.Object);
	}
}

void AMeshLabScene::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Cube — built directly into our own PMC component, centered on the actor origin.
	if (CubeMesh)
	{
		CubeMesh->ClearAllMeshSections();

		TArray<FVector> Vertices;
		TArray<int32> Triangles;
		TArray<FVector> Normals;
		TArray<FVector2D> UV0;
		TArray<FLinearColor> Colors;
		AMeshLabSurface::BuildCube(CubeSize, Vertices, Triangles, Normals, UV0, Colors);

		const TArray<FProcMeshTangent> Tangents;
		CubeMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, Colors, Tangents, /*bCreateCollision=*/false);

		if (CubeMaterial)
		{
			CubeMesh->SetMaterial(0, CubeMaterial);
		}
	}

	// Lit witnesses — flank the cube on the X axis so the dirlight's shading is obvious.
	if (CylinderWitness)
	{
		CylinderWitness->SetRelativeLocation(FVector(-LitWitnessXOffset, 0.0f, 0.0f));
	}
	if (ConeWitness)
	{
		ConeWitness->SetRelativeLocation(FVector(LitWitnessXOffset, 0.0f, 0.0f));
	}

	if (SunLight)
	{
		SunLight->SetRelativeRotation(DirectionalLightRotation);
		SunLight->SetIntensity(DirectionalLightIntensity);
	}

	if (SkyLight)
	{
		SkyLight->SetIntensity(SkyLightIntensity);
	}
}
