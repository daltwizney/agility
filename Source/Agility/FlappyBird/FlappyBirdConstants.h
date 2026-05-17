#pragma once

#include "CoreMinimal.h"

// World units are Unreal centimeters (1 m == 100 cm). The kollie reference was
// authored in meters; values here are the kollie numbers ×100. Axes map as:
//   kollie X  →  UE X  (scroll axis; pipes spawn at +X, despawn at -X)
//   kollie Y  →  UE Z  (vertical / up)
//   kollie Z  →  UE Y  (depth; camera sits on +Y looking back toward -Y)
namespace FlappyBirdConstants
{
	// --- World bounds (Z up)
	constexpr float WorldFloorZ   = -450.0f;
	constexpr float WorldCeilingZ =  450.0f;
	constexpr float BirdX         =    0.0f;
	constexpr float BirdRadius    =   45.0f;
	constexpr float BirdSpawnZ    =   50.0f;   // kollie: 0.5 m

	// --- Bird physics
	constexpr float Gravity       = -2200.0f;  // cm/s^2 (already signed; vy += Gravity * dt)
	constexpr float FlapVelocity  =   850.0f;  // cm/s
	constexpr float MaxFallSpeed  = -1400.0f;  // cm/s (clamp floor)
	constexpr float PitchUpDeg    =    28.0f;
	constexpr float PitchDownDeg  =   -55.0f;

	// --- Pipes
	constexpr float PipeSpawnX         =  1300.0f;
	constexpr float PipeDespawnX       = -1300.0f;
	constexpr float PipeScrollSpeed    =   420.0f;
	constexpr float PipeSpawnInterval  =     1.85f;
	constexpr float InitialGraceDelay  =     0.65f;
	constexpr float PipeGapHeight      =   310.0f;
	constexpr float PipeGapZRange      =   190.0f;   // ± from Z=0
	constexpr float PipeColumnHalfWidth=    55.0f;
	constexpr float PipeColumnHalfDepth=    55.0f;
	constexpr float PipeColumnTopZ     =   650.0f;
	constexpr float PipeColumnBottomZ  =  -650.0f;
	constexpr float PipeCapHalfHeight  =    18.0f;   // shaft-end cap thickness ≈ 0.18 m * 2 / 2
	constexpr float PipeCapWidthMul    =     1.55f;  // cap is wider than the shaft (mario-pipe-y)

	// --- Camera (world-locked, looking -Y at the playfield)
	constexpr float CameraOffsetX = 150.0f;   // kollie eye.x = 1.5
	constexpr float CameraOffsetZ =  50.0f;   // kollie eye.y = 0.5
	constexpr float CameraDistance = 1600.0f; // kollie eye.z = 16 (portrait)
	constexpr float CameraTargetZ =  50.0f;   // kollie target.y = 0.5
	constexpr float CameraFovDeg  =  60.0f;   // portrait FOV
}
