#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeshLabScene.generated.h"

class UProceduralMeshComponent;
class UStaticMeshComponent;
class UDirectionalLightComponent;
class USkyLightComponent;
class UMaterialInterface;

/**
 * Scene-director actor: hosts a procedural cube + directional light + sky light as direct
 * components on a single actor. Drop one in an empty level — every part is selectable
 * individually in the Details panel's component tree, and tweaking a property re-runs
 * OnConstruction so the cube/lights update live.
 *
 * Components (not child actors) by design: ChildActorComponent's spawn-and-configure
 * timing is unreliable in editor construction-script context, and direct components
 * solve the "everything is one selectable thing in the outliner" UX in the same stroke.
 */
UCLASS()
class AGILITY_API AMeshLabScene : public AActor
{
	GENERATED_BODY()

public:
	AMeshLabScene();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere, Category = "MeshLab|Scene|Cube")
	TObjectPtr<UMaterialInterface> CubeMaterial;

	UPROPERTY(EditAnywhere, Category = "MeshLab|Scene|Cube", meta = (ClampMin = "1.0"))
	float CubeSize = 100.0f;

	UPROPERTY(EditAnywhere, Category = "MeshLab|Scene|Lighting", meta = (ClampMin = "0.0"))
	float DirectionalLightIntensity = 5.0f;

	UPROPERTY(EditAnywhere, Category = "MeshLab|Scene|Lighting")
	FRotator DirectionalLightRotation = FRotator(-45.0f, -30.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "MeshLab|Scene|Lighting", meta = (ClampMin = "0.0"))
	float SkyLightIntensity = 1.0f;

	// Lit-shape witnesses — engine basic-shape static meshes flanking the cube on the X axis,
	// using their default lit materials so the dirlight's effect is visible at a glance.
	UPROPERTY(EditAnywhere, Category = "MeshLab|Scene|Lit Witnesses", meta = (ClampMin = "0.0"))
	float LitWitnessXOffset = 300.0f;

	UPROPERTY(VisibleAnywhere, Category = "MeshLab|Scene")
	TObjectPtr<UProceduralMeshComponent> CubeMesh;

	UPROPERTY(VisibleAnywhere, Category = "MeshLab|Scene")
	TObjectPtr<UStaticMeshComponent> CylinderWitness;

	UPROPERTY(VisibleAnywhere, Category = "MeshLab|Scene")
	TObjectPtr<UStaticMeshComponent> ConeWitness;

	UPROPERTY(VisibleAnywhere, Category = "MeshLab|Scene")
	TObjectPtr<UDirectionalLightComponent> SunLight;

	UPROPERTY(VisibleAnywhere, Category = "MeshLab|Scene")
	TObjectPtr<USkyLightComponent> SkyLight;
};
