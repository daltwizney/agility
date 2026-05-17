#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlappyBackground.generated.h"

class UBoxComponent;
class UDirectionalLightComponent;
class UProceduralMeshComponent;
class USceneComponent;
class UStaticMeshComponent;

// Procedural scenery for the FlappyBird port: a single ground plane, the
// floor + ceiling colliders that kill the bird if it falls or flies off-screen,
// five low-poly mountains in a row at the play-plane back, twelve stars
// scattered overhead, a glowing moon, and the directional moonlight that lights
// the playfield. Mirrors the static `Sky` subtree in kollie's FlappyBirdScene.kt.
//
// The actor is meant to be spawned by the game mode — no editor placement
// required. Mesh geometry and per-mesh emissive MIDs are built in OnConstruction
// so it's also drag-into-level friendly during iteration.
UCLASS()
class AGILITY_API AFlappyBackground : public AActor
{
	GENERATED_BODY()

public:
	AFlappyBackground();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBG")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBG")
	TObjectPtr<UStaticMeshComponent> GroundMesh;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBG")
	TObjectPtr<UStaticMeshComponent> MoonMesh;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBG")
	TObjectPtr<UProceduralMeshComponent> MountainsMesh;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBG")
	TObjectPtr<UDirectionalLightComponent> MoonLight;

	// Hemispheric fake-ambient — two directional fills that together give the
	// scene the "purple sky / dim purple ground" wash from kollie's
	// ambientSky / ambientGround constants. Cheaper than a SkyLight + cubemap
	// asset and entirely zero-asset.
	UPROPERTY(VisibleAnywhere, Category = "FlappyBG")
	TObjectPtr<UDirectionalLightComponent> SkyFillUpLight;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBG")
	TObjectPtr<UDirectionalLightComponent> SkyFillDownLight;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBG|Collision")
	TObjectPtr<UBoxComponent> FloorCollider;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBG|Collision")
	TObjectPtr<UBoxComponent> CeilingCollider;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBG")
	TArray<TObjectPtr<UStaticMeshComponent>> StarMeshes;

private:
	void BuildMountainsMesh();
};
