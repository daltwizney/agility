#include "FlappyBird.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "FlappyBackground.h"
#include "FlappyBirdConstants.h"
#include "FlappyBirdGameMode.h"
#include "FlappyNeonMaterials.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

using namespace FlappyBirdConstants;

namespace
{
	// kollie (X, Yvert, Zdepth) → UE (X, Y=Zdepth, Z=Yvert), scale ×100 (m → cm).
	FVector KollieMeters(float X, float YVert, float ZDepth)
	{
		return FVector(X * 100.0f, ZDepth * 100.0f, YVert * 100.0f);
	}
	FVector KollieScale(float X, float YVert, float ZDepth)
	{
		return FVector(X, ZDepth, YVert);
	}

	// Engine /BasicShapes meshes referenced by the bird parts.
	// Sphere = 100 cm diameter, Cone = 100 cm tall / 100 cm base diameter, Cube = 100 cm side.
	//
	// LoadObject (not ConstructorHelpers::FObjectFinder) because these are called from
	// OnConstruction — FObjectFinder asserts it's used during the actor's constructor only.
	UStaticMesh* LoadBasicSphere() { return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")); }
	UStaticMesh* LoadBasicCone()   { return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));     }
	UStaticMesh* LoadBasicCube()   { return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));     }
}

AFlappyBird::AFlappyBird()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));
	Collider->SetupAttachment(Root);
	Collider->SetSphereRadius(BirdRadius);
	Collider->SetCollisionProfileName(TEXT("OverlapAll"));
	Collider->SetGenerateOverlapEvents(true);
	Collider->OnComponentBeginOverlap.AddDynamic(this, &AFlappyBird::OnColliderOverlap);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Root);
	Camera->SetFieldOfView(CameraFovDeg);
	// World-lock: camera ignores the pawn's transform so the bird can fly up/down within the frame.
	Camera->SetUsingAbsoluteLocation(true);
	Camera->SetUsingAbsoluteRotation(true);
	// Bloom — tuned to match kollie's "every neon edge has a halo" look. The kollie
	// scene runs intensity=0.32 + highlight=1000, which in UE-equivalent terms means
	// "low gain, but let *everything* bright contribute". So:
	//   - Threshold = -1.0 disables the brightness gate so soft emissive falloff also
	//     contributes (the gate at default 1.0 chops the halo's outer fade and makes
	//     pipes / stars look pasted-on).
	//   - Intensity = 3.0 to give the halos visible reach.
	//   - SizeScale = 8.0 widens the blur kernel so the halos extend further from
	//     the pipe / moon / wing edge, matching the dreamy soft-glow in the kollie
	//     reference rather than the tight contour bloom UE gives by default.
	Camera->PostProcessBlendWeight = 1.0f;
	Camera->PostProcessSettings.bOverride_BloomIntensity = true;
	Camera->PostProcessSettings.BloomIntensity = 3.0f;
	Camera->PostProcessSettings.bOverride_BloomThreshold = true;
	Camera->PostProcessSettings.BloomThreshold = -1.0f;
	Camera->PostProcessSettings.bOverride_BloomSizeScale = true;
	Camera->PostProcessSettings.BloomSizeScale = 8.0f;
	// ACES tonemap is UE's default — kollie also requests "aces". Nothing to override.

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// --- Mesh components. Allocate now; the SetStaticMesh + transform + MID
	// happens in OnConstruction so editor-time tweaks rebuild cleanly.
	auto MakeMesh = [&](FName Name) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* M = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		M->SetupAttachment(Root);
		M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		M->SetCastShadow(false);
		return M;
	};
	BodyMesh   = MakeMesh(TEXT("BodyMesh"));
	BeakMesh   = MakeMesh(TEXT("BeakMesh"));
	CrestMesh  = MakeMesh(TEXT("CrestMesh"));
	TailMesh   = MakeMesh(TEXT("TailMesh"));
	EyeLMesh   = MakeMesh(TEXT("EyeLMesh"));
	EyeRMesh   = MakeMesh(TEXT("EyeRMesh"));
	PupilLMesh = MakeMesh(TEXT("PupilLMesh"));
	PupilRMesh = MakeMesh(TEXT("PupilRMesh"));

	WingPivotL = CreateDefaultSubobject<USceneComponent>(TEXT("WingPivotL"));
	WingPivotL->SetupAttachment(Root);
	WingPivotR = CreateDefaultSubobject<USceneComponent>(TEXT("WingPivotR"));
	WingPivotR->SetupAttachment(Root);

	WingMeshL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingMeshL"));
	WingMeshL->SetupAttachment(WingPivotL);
	WingMeshL->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WingMeshL->SetCastShadow(false);

	WingMeshR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingMeshR"));
	WingMeshR->SetupAttachment(WingPivotR);
	WingMeshR->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WingMeshR->SetCastShadow(false);
}

void AFlappyBird::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (Collider) { Collider->SetSphereRadius(BirdRadius); }

	UStaticMesh* Sphere = LoadBasicSphere();
	UStaticMesh* Cone   = LoadBasicCone();
	UStaticMesh* Cube   = LoadBasicCube();

	auto Setup = [](UStaticMeshComponent* M, UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale,
		const FlappyNeonPalette::FNeonSpec& Spec)
	{
		if (!M) return;
		if (Mesh) M->SetStaticMesh(Mesh);
		M->SetRelativeLocation(Loc);
		M->SetRelativeRotation(Rot);
		M->SetRelativeScale3D(Scale);
		if (UMaterialInstanceDynamic* MID = FlappyNeonMaterials::MakeMID(M, Spec))
		{
			M->SetMaterial(0, MID);
		}
	};

	// --- Body: sphere flattened slightly in vertical (kollie scale 0.95, 0.85, 0.95).
	Setup(BodyMesh, Sphere,
		FVector(0.0f, 0.0f, 0.0f), FRotator::ZeroRotator,
		KollieScale(0.95f, 0.85f, 0.95f),
		FlappyNeonPalette::BirdBody);

	// --- Beak: cone pointing along +X (forward). Engine Cone apex is +Z, so
	// pitching -90° around Y reorients to +X. Scale tall+narrow then rotates.
	Setup(BeakMesh, Cone,
		KollieMeters(0.50f, -0.05f, 0.0f), FRotator(-90.0f, 0.0f, 0.0f),
		FVector(0.18f, 0.18f, 0.45f),
		FlappyNeonPalette::BirdBeak);

	// --- Crest: small cone on top of head, tilted -12° forward (kollie Z-roll
	// = UE Y-rotation = Pitch).
	Setup(CrestMesh, Cone,
		KollieMeters(0.05f, 0.42f, 0.0f), FRotator(-12.0f, 0.0f, 0.0f),
		FVector(0.12f, 0.12f, 0.40f),
		FlappyNeonPalette::BirdCrest);

	// --- Tail: cone pointing -X (backward). Pitch +90 flips the cone's apex
	// from +Z to -X.
	Setup(TailMesh, Cone,
		KollieMeters(-0.50f, 0.05f, 0.0f), FRotator(90.0f, 0.0f, 0.0f),
		FVector(0.16f, 0.16f, 0.42f),
		FlappyNeonPalette::BirdTail);

	// --- Eyes + pupils. Left = +Y (camera side), Right = -Y. Scales are uniform
	// — the engine Sphere is 100 cm, so 0.20 → 20 cm eye, 0.10 → 10 cm pupil.
	Setup(EyeLMesh,   Sphere, KollieMeters(0.28f, 0.18f,  0.26f), FRotator::ZeroRotator, FVector(0.20f), FlappyNeonPalette::BirdEyeWhite);
	Setup(PupilLMesh, Sphere, KollieMeters(0.36f, 0.19f,  0.30f), FRotator::ZeroRotator, FVector(0.10f), FlappyNeonPalette::BirdPupil);
	Setup(EyeRMesh,   Sphere, KollieMeters(0.28f, 0.18f, -0.26f), FRotator::ZeroRotator, FVector(0.20f), FlappyNeonPalette::BirdEyeWhite);
	Setup(PupilRMesh, Sphere, KollieMeters(0.36f, 0.19f, -0.30f), FRotator::ZeroRotator, FVector(0.10f), FlappyNeonPalette::BirdPupil);

	// --- Wing pivots — at the shoulder. Mesh is offset further out so flap
	// rotation around the pivot swings the tip in a real arc.
	if (WingPivotL) WingPivotL->SetRelativeLocation(KollieMeters(-0.05f, 0.0f,  0.38f));
	if (WingPivotR) WingPivotR->SetRelativeLocation(KollieMeters(-0.05f, 0.0f, -0.38f));
	Setup(WingMeshL, Cube,
		KollieMeters(-0.10f, 0.0f,  0.38f) - KollieMeters(-0.05f, 0.0f,  0.38f),
		FRotator::ZeroRotator, KollieScale(0.65f, 0.10f, 0.85f), FlappyNeonPalette::BirdWing);
	Setup(WingMeshR, Cube,
		KollieMeters(-0.10f, 0.0f, -0.38f) - KollieMeters(-0.05f, 0.0f, -0.38f),
		FRotator::ZeroRotator, KollieScale(0.65f, 0.10f, 0.85f), FlappyNeonPalette::BirdWing);

	// Camera position once we know where the bird spawned. World-locked so this
	// stays put even as the bird moves vertically.
	if (Camera)
	{
		const FVector ActorLoc = GetActorLocation();
		Camera->SetWorldLocation(FVector(ActorLoc.X + CameraOffsetX, CameraDistance, CameraOffsetZ));
		const FVector LookFrom = Camera->GetComponentLocation();
		const FVector LookAt   = FVector(ActorLoc.X, 0.0f, CameraTargetZ);
		Camera->SetWorldRotation((LookAt - LookFrom).Rotation());
	}
}

void AFlappyBird::BeginPlay()
{
	Super::BeginPlay();

	// Bird spawns at BirdSpawnZ — but the level may have placed a PlayerStart
	// elsewhere. Override on BeginPlay so the kollie spawn pose is canonical.
	SetActorLocation(FVector(BirdX, 0.0f, BirdSpawnZ));
	SetActorRotation(FRotator::ZeroRotator);

	if (Camera)
	{
		Camera->SetWorldLocation(FVector(GetActorLocation().X + CameraOffsetX, CameraDistance, CameraOffsetZ));
		const FVector LookFrom = Camera->GetComponentLocation();
		const FVector LookAt   = FVector(GetActorLocation().X, 0.0f, CameraTargetZ);
		Camera->SetWorldRotation((LookAt - LookFrom).Rotation());
	}

	// Audio. Paths match the user's import locations under Plugins/Agility/Content/Audio.
	// Asset names default to the file basename without extension; we re-derive them by file path.
	SfxFlap  = LoadObject<USoundBase>(nullptr, TEXT("/Agility/Audio/SFX/space-shooter/sfx_shieldUp.sfx_shieldUp"));
	SfxScore = LoadObject<USoundBase>(nullptr, TEXT("/Agility/Audio/SFX/space-shooter/sfx_twoTone.sfx_twoTone"));
	SfxHit   = LoadObject<USoundBase>(nullptr, TEXT("/Agility/Audio/SFX/space-shooter/sfx_lose.sfx_lose"));
}

void AFlappyBird::ResetForReady()
{
	State = EBirdState::Idle;
	VelocityZ = 0.0f;
	FlapBoostTimer = 0.0f;
	WingPhase = 0.0f;
	IdleTimeAccum = 0.0f;
	SetActorLocation(FVector(BirdX, 0.0f, BirdSpawnZ));
	SetActorRotation(FRotator::ZeroRotator);
}

void AFlappyBird::StartFlying()
{
	if (State == EBirdState::Dead) return;
	State = EBirdState::Flying;
	Flap();
}

void AFlappyBird::OnFlapPressed()
{
	UWorld* World = GetWorld();
	AFlappyBirdGameMode* GM = World ? World->GetAuthGameMode<AFlappyBirdGameMode>() : nullptr;
	if (!GM)
	{
		// No game mode — just flap unconditionally (handy when debugging the
		// pawn in an empty test level).
		Flap();
		return;
	}

	switch (GM->GetPhase())
	{
	case EFlappyPhase::Ready:
		GM->BeginPlayPhase();
		StartFlying();
		break;
	case EFlappyPhase::Playing:
		Flap();
		break;
	case EFlappyPhase::Dead:
		GM->RequestRestart();
		break;
	}
}

void AFlappyBird::Flap()
{
	if (State == EBirdState::Dead) return;
	VelocityZ = FlapVelocity;
	FlapBoostTimer = 0.18f;
	PlaySoundOnce(SfxFlap, 0.45f);
}

void AFlappyBird::Die()
{
	if (State == EBirdState::Dead) return;
	State = EBirdState::Dead;
	// Tiny upward kick so the falling arc reads visually.
	VelocityZ = FMath::Abs(VelocityZ) * 0.3f + 200.0f;
	PlaySoundOnce(SfxHit, 0.65f);
	if (UWorld* World = GetWorld())
	{
		if (AFlappyBirdGameMode* GM = World->GetAuthGameMode<AFlappyBirdGameMode>())
		{
			GM->NotifyBirdDied();
		}
	}
}

void AFlappyBird::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	WingPhase += DeltaSeconds;

	switch (State)
	{
	case EBirdState::Idle:    TickIdle(DeltaSeconds);    break;
	case EBirdState::Flying:  TickFlying(DeltaSeconds);  break;
	case EBirdState::Dead:    TickDead(DeltaSeconds);    break;
	}
}

void AFlappyBird::TickIdle(float Dt)
{
	IdleTimeAccum += Dt;
	const float Bob = FMath::Sin(IdleTimeAccum * 2.0f * PI * 1.4f) * 18.0f;
	SetActorLocation(FVector(BirdX, 0.0f, BirdSpawnZ + Bob));
	SetActorRotation(FRotator::ZeroRotator);
	AnimateWings(Dt, /*RateHz=*/5.0f, /*AmplitudeDeg=*/22.0f);
}

void AFlappyBird::TickFlying(float Dt)
{
	VelocityZ += Gravity * Dt;
	VelocityZ = FMath::Max(VelocityZ, MaxFallSpeed);

	FVector L = GetActorLocation();
	L.Z += VelocityZ * Dt;
	SetActorLocation(FVector(BirdX, 0.0f, L.Z));

	// Pitch lerp — kollie maps (vy + 8 m/s) / 16 onto [-55°, +28°]. In cm/s that's
	// (vy + 800) / 1600 over the same [0, 1] range.
	const float T = FMath::Clamp((VelocityZ + 800.0f) / 1600.0f, 0.0f, 1.0f);
	const float TargetPitch = PitchDownDeg + (PitchUpDeg - PitchDownDeg) * T;
	const float CurPitch = GetActorRotation().Pitch;
	const float NextPitch = CurPitch + (TargetPitch - CurPitch) * FMath::Min(8.0f * Dt, 1.0f);
	SetActorRotation(FRotator(NextPitch, 0.0f, 0.0f));

	const float Rate = (FlapBoostTimer > 0.0f) ? 14.0f : 6.0f;
	if (FlapBoostTimer > 0.0f) { FlapBoostTimer -= Dt; }
	AnimateWings(Dt, Rate, /*AmplitudeDeg=*/38.0f);
}

void AFlappyBird::TickDead(float Dt)
{
	VelocityZ += Gravity * Dt;
	VelocityZ = FMath::Max(VelocityZ, MaxFallSpeed);
	FVector L = GetActorLocation();
	L.Z = FMath::Max(L.Z + VelocityZ * Dt, WorldFloorZ + 10.0f);
	SetActorLocation(FVector(BirdX, 0.0f, L.Z));
	// Ragdoll spin around the pitch axis at 180°/s.
	SetActorRotation(GetActorRotation() + FRotator(180.0f * Dt, 0.0f, 0.0f));
	// Pin wings out flat.
	if (WingPivotL) WingPivotL->SetRelativeRotation(FRotator(0.0f, 0.0f, -65.0f));
	if (WingPivotR) WingPivotR->SetRelativeRotation(FRotator(0.0f, 0.0f,  65.0f));
}

void AFlappyBird::AnimateWings(float Dt, float RateHz, float AmplitudeDeg)
{
	const float Phase = FMath::Sin(WingPhase * 2.0f * PI * RateHz);
	constexpr float RestDeg = 6.0f;
	const float Roll = RestDeg + Phase * AmplitudeDeg;
	// Mirrored signs keep both wing tips moving the same direction in world Z —
	// "both wings down together" rather than alternating paddles.
	if (WingPivotL) WingPivotL->SetRelativeRotation(FRotator(0.0f, 0.0f,  Roll));
	if (WingPivotR) WingPivotR->SetRelativeRotation(FRotator(0.0f, 0.0f, -Roll));
}

void AFlappyBird::OnColliderOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Pipes handle their own hit/score notifications (they know which collider
	// is which). The bird's listener only catches background floor/ceiling.
	if (OtherActor && OtherActor->IsA(AFlappyBackground::StaticClass()))
	{
		if (State == EBirdState::Flying)
		{
			Die();
		}
	}
}

void AFlappyBird::PlaySoundOnce(USoundBase* Sound, float Volume)
{
	if (!Sound) return;
	UGameplayStatics::PlaySound2D(this, Sound, Volume);
}

void AFlappyBird::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (!PlayerInputComponent) return;
	// Action name + SpaceBar / LMB mapping are shipped by the plugin in
	// Plugins/Agility/Config/DefaultInput.ini. Namespaced "Agility.FlappyBird.*"
	// so it can't collide with a host project's own action names.
	PlayerInputComponent->BindAction(TEXT("Agility.FlappyBird.Flap"), IE_Pressed, this, &AFlappyBird::OnFlapPressed);
}
