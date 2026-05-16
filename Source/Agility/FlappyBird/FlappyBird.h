#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FlappyBird.generated.h"

class UProceduralMeshComponent;
class UCameraComponent;
class USceneComponent;
class USphereComponent;
class UMaterialInterface;

UCLASS()
class AGILITY_API AFlappyBird : public APawn
{
	GENERATED_BODY()

public:
	AFlappyBird();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBird")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBird")
	TObjectPtr<UProceduralMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBird")
	TObjectPtr<UProceduralMeshComponent> WingMesh;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBird")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, Category = "FlappyBird")
	TObjectPtr<USphereComponent> Collider;

	// --- Bird shape (cm)
	UPROPERTY(EditAnywhere, Category = "FlappyBird|Shape")
	FVector BodyRadii = FVector(55.0f, 30.0f, 38.0f);

	UPROPERTY(EditAnywhere, Category = "FlappyBird|Shape")
	float BeakLength = 28.0f;

	UPROPERTY(EditAnywhere, Category = "FlappyBird|Shape")
	float BeakRadius = 10.0f;

	UPROPERTY(EditAnywhere, Category = "FlappyBird|Shape")
	float EyeRadius = 5.5f;

	UPROPERTY(EditAnywhere, Category = "FlappyBird|Shape")
	float WingLength = 55.0f;

	UPROPERTY(EditAnywhere, Category = "FlappyBird|Shape")
	float WingTipHeight = 18.0f;

	UPROPERTY(EditAnywhere, Category = "FlappyBird|Shape", meta = (ClampMin = "4", ClampMax = "48"))
	int32 SphereSegments = 18;

	UPROPERTY(EditAnywhere, Category = "FlappyBird|Shape", meta = (ClampMin = "3", ClampMax = "24"))
	int32 SphereRings = 12;

	// --- Animation
	UPROPERTY(EditAnywhere, Category = "FlappyBird|Animation")
	float FlapFrequencyHz = 6.0f;

	UPROPERTY(EditAnywhere, Category = "FlappyBird|Animation")
	float FlapAmplitudeDeg = 45.0f;

	UPROPERTY(EditAnywhere, Category = "FlappyBird|Animation")
	float FlapRestDeg = 10.0f;

	UPROPERTY(EditAnywhere, Category = "FlappyBird|Animation")
	float FlapBoostFrequencyAddHz = 10.0f;

	UPROPERTY(EditAnywhere, Category = "FlappyBird|Animation", meta = (ClampMin = "0.0"))
	float FlapBoostDuration = 0.5f;

	// --- Physics (cm/s, cm/s^2)
	UPROPERTY(EditAnywhere, Category = "FlappyBird|Physics")
	float Gravity = 2200.0f;

	UPROPERTY(EditAnywhere, Category = "FlappyBird|Physics")
	float FlapImpulse = 700.0f;

	UPROPERTY(EditAnywhere, Category = "FlappyBird|Physics")
	float MaxFallSpeed = 1400.0f;

	// --- Camera
	UPROPERTY(EditAnywhere, Category = "FlappyBird|Camera")
	float CameraDistance = 900.0f;

	// --- Collision
	UPROPERTY(EditAnywhere, Category = "FlappyBird|Collision", meta = (ClampMin = "1.0"))
	float CollisionRadius = 35.0f;

private:
	void Flap();
	void BuildBody();
	void BuildWing();
	void ApplySectionColor(UProceduralMeshComponent* Mesh, int32 SectionIndex, const FLinearColor& Color);

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ColoredParentMaterial;

	float VerticalVelocity = 0.0f;
	float FlapPhase = 0.0f;
	float FlapBoostRemaining = 0.0f;
};
