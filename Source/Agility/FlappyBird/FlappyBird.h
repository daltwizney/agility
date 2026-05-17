#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FlappyBird.generated.h"

class UCameraComponent;
class UPrimitiveComponent;
class USceneComponent;
class USoundBase;
class USphereComponent;
class UStaticMeshComponent;

UENUM()
enum class EBirdState : uint8
{
	Idle,    // Pre-flap, bobs in place. First Flap input transitions to Flying.
	Flying,  // Gravity + flap + pitch + wing-flap animation.
	Dead,    // Ragdoll spin + gravity; wings pinned out. Click → restart via GameMode.
};

// Procedural bird: cube-sphere body + cone beak/crest/tail + spherical eyes
// (white + black pupil) + two flap-pivot wings (cubes). Mirrors kollie's
// FlappyBirdController exactly — same physics constants, same wing animation
// curve, same pitch lerp, same dead-fall spin.
//
// Camera lives on the pawn but is world-locked (absolute location/rotation) so
// the bird can move up/down within the frame. Bloom is configured in C++ on
// the camera's PostProcessSettings; no post-process volume asset needed.
//
// Collision: the sphere collider listens for overlaps with the FlappyBackground
// floor/ceiling kill volumes and notifies the GameMode. Pipe / score-gate
// overlaps are owned by APipePair itself (it knows which is which).
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

	// --- Component tree
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<USceneComponent> Root;
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<USphereComponent> Collider;
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<UStaticMeshComponent> BodyMesh;
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<UStaticMeshComponent> BeakMesh;
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<UStaticMeshComponent> CrestMesh;
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<UStaticMeshComponent> TailMesh;
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<UStaticMeshComponent> EyeLMesh;
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<UStaticMeshComponent> EyeRMesh;
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<UStaticMeshComponent> PupilLMesh;
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<UStaticMeshComponent> PupilRMesh;

	// Wing pivots are SceneComponents — flapping rotates the pivot; the visual
	// cube is a child, offset outward, so its tip swings in a real arc.
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<USceneComponent> WingPivotL;
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<USceneComponent> WingPivotR;
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<UStaticMeshComponent> WingMeshL;
	UPROPERTY(VisibleAnywhere, Category = "Bird") TObjectPtr<UStaticMeshComponent> WingMeshR;

	// --- State (read by GameMode for HUD / phase decisions)
	EBirdState GetState() const { return State; }

	// Called by GameMode on RequestRestart — moves the bird back to spawn, resets velocity/state.
	void ResetForReady();

	// Called by GameMode when transitioning READY→PLAYING (in addition to any flap impulse).
	void StartFlying();

	UFUNCTION()
	void OnColliderOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void OnFlapPressed();
	void Flap();
	void Die();

	void TickIdle(float Dt);
	void TickFlying(float Dt);
	void TickDead(float Dt);
	void AnimateWings(float Dt, float RateHz, float AmplitudeDeg);

	void PlaySoundOnce(USoundBase* Sound, float Volume);

	EBirdState State = EBirdState::Idle;
	float VelocityZ = 0.0f;
	float FlapBoostTimer = 0.0f;
	float WingPhase = 0.0f;
	float IdleTimeAccum = 0.0f;

	// Cached SFX (loaded on BeginPlay via LoadObject — see .cpp for paths).
	UPROPERTY() TObjectPtr<USoundBase> SfxFlap;
	UPROPERTY() TObjectPtr<USoundBase> SfxScore;
	UPROPERTY() TObjectPtr<USoundBase> SfxHit;

	// Made visible to GameMode so it can play the score sound when a pipe reports a score.
	friend class AFlappyBirdGameMode;
};
