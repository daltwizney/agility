#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class UObject;
class UProceduralMeshComponent;

// Centralized neon palette for the FlappyBird port. Mirrors the per-material
// values from kollie's FlappyBirdScene.kt — every entry is a flat base color
// plus the emissive triple the kollie shader fed into its emissive output.
//
// All MIDs are derived from the single base material
// `/Agility/Materials/M_AgilityNeonEmissive` (authored once in editor — see
// Docs/tutorials/flappybird/00-scope.md Part 6 for the asset spec).
namespace FlappyNeonPalette
{
	struct FNeonSpec
	{
		FLinearColor BaseColor;
		FLinearColor EmissiveColor;   // RGB only; A unused
		float        EmissiveIntensity;
	};

	// Background
	inline const FNeonSpec Ground   { FLinearColor(0.04f, 0.02f, 0.07f, 1.0f), FLinearColor(0.5f, 0.15f, 0.6f), 0.06f };
	inline const FNeonSpec Mountain { FLinearColor(0.02f, 0.01f, 0.05f, 1.0f), FLinearColor(0.6f, 0.2f, 1.0f),  0.18f };
	inline const FNeonSpec Star     { FLinearColor(0.05f, 0.05f, 0.06f, 1.0f), FLinearColor(3.5f, 3.4f, 4.0f),  1.0f };
	inline const FNeonSpec Moon     { FLinearColor(0.05f, 0.05f, 0.07f, 1.0f), FLinearColor(2.4f, 2.6f, 3.4f),  1.0f };

	// Bird
	inline const FNeonSpec BirdBody    { FLinearColor(0.18f, 0.12f, 0.04f, 1.0f), FLinearColor(3.6f, 2.4f, 0.4f), 0.45f };
	inline const FNeonSpec BirdBeak    { FLinearColor(0.10f, 0.04f, 0.02f, 1.0f), FLinearColor(3.8f, 1.2f, 0.2f), 0.9f  };
	inline const FNeonSpec BirdWing    { FLinearColor(0.06f, 0.02f, 0.10f, 1.0f), FLinearColor(3.4f, 0.8f, 3.6f), 0.7f  };
	inline const FNeonSpec BirdCrest   { FLinearColor(0.02f, 0.06f, 0.10f, 1.0f), FLinearColor(0.3f, 3.4f, 4.0f), 1.0f  };
	inline const FNeonSpec BirdTail    { FLinearColor(0.08f, 0.02f, 0.10f, 1.0f), FLinearColor(3.2f, 0.9f, 3.4f), 0.8f  };
	inline const FNeonSpec BirdEyeWhite{ FLinearColor(0.08f, 0.08f, 0.10f, 1.0f), FLinearColor(3.2f, 3.2f, 3.4f), 0.5f  };
	inline const FNeonSpec BirdPupil   { FLinearColor(0.01f, 0.01f, 0.02f, 1.0f), FLinearColor(0.0f, 0.0f, 0.0f), 0.0f  };

	// Pipes — alternating cyan / magenta. Shafts have lower intensity than caps
	// so the gap-end "lid" reads as the brightest band on each pipe.
	inline const FNeonSpec PipeShaftCyan   { FLinearColor(0.02f, 0.06f, 0.08f, 1.0f), FLinearColor(0.15f, 2.4f, 3.0f), 0.55f };
	inline const FNeonSpec PipeCapCyan     { FLinearColor(0.02f, 0.08f, 0.10f, 1.0f), FLinearColor(0.4f,  3.4f, 4.2f), 1.1f  };
	inline const FNeonSpec PipeShaftMagenta{ FLinearColor(0.08f, 0.02f, 0.08f, 1.0f), FLinearColor(3.2f,  0.4f, 2.6f), 0.55f };
	inline const FNeonSpec PipeCapMagenta  { FLinearColor(0.10f, 0.02f, 0.10f, 1.0f), FLinearColor(4.0f,  0.6f, 3.4f), 1.1f  };
}

namespace FlappyNeonMaterials
{
	// Load the shared base material `M_AgilityNeonEmissive`. Cached on the
	// returned pointer — repeated calls are cheap. Returns nullptr if the
	// asset isn't present (user hasn't created it yet); callers should log
	// once and fall back gracefully.
	UMaterialInterface* GetBaseMaterial();

	// Create a MID from the base material with the spec applied. `Outer`
	// is typically the component / actor that will own the MID.
	UMaterialInstanceDynamic* MakeMID(UObject* Outer, const FlappyNeonPalette::FNeonSpec& Spec);

	// Apply a spec to a single procedural-mesh section. No-op if base material
	// is missing (warning logged once on first failure).
	void ApplySectionSpec(
		UProceduralMeshComponent* Mesh,
		int32 SectionIndex,
		const FlappyNeonPalette::FNeonSpec& Spec);
}
