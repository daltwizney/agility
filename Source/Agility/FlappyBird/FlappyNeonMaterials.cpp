#include "FlappyNeonMaterials.h"

#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFlappyNeon, Log, All);

namespace
{
	// Ships with the plugin under Content/Materials/. Identifier is namespace-unique because
	// UE's unity build concatenates source files into a single translation unit, and a bare
	// `GBaseMaterialPath` here would collide with the same name in `AgilityVideoQuad.cpp`.
	// Prefix keeps both happy.
	const TCHAR* const GFlappyNeonBaseMaterialPath = TEXT("/Agility/Materials/M_AgilityNeonEmissive.M_AgilityNeonEmissive");

	bool GFlappyNeonHasWarnedMissingBase = false;
}

namespace FlappyNeonMaterials
{
	UMaterialInterface* GetBaseMaterial()
	{
		// LoadObject is cheap on the second hit (already in memory), so a static
		// cache only saves a hash lookup — skipped for simplicity.
		UMaterialInterface* Base = LoadObject<UMaterial>(nullptr, GFlappyNeonBaseMaterialPath);
		if (!Base && !GFlappyNeonHasWarnedMissingBase)
		{
			GFlappyNeonHasWarnedMissingBase = true;
			UE_LOG(LogFlappyNeon, Warning,
				TEXT("FlappyNeonMaterials: base material missing at %s. Bird/pipes/background will appear unshaded.")
				TEXT(" Verify the Agility plugin's Content/Materials/ is intact and the plugin is enabled."),
				GFlappyNeonBaseMaterialPath);
		}
		return Base;
	}

	UMaterialInstanceDynamic* MakeMID(UObject* Outer, const FlappyNeonPalette::FNeonSpec& Spec)
	{
		UMaterialInterface* Base = GetBaseMaterial();
		if (!Base)
		{
			return nullptr;
		}
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, Outer);
		if (!MID)
		{
			return nullptr;
		}
		MID->SetVectorParameterValue(TEXT("BaseColor"), Spec.BaseColor);
		MID->SetVectorParameterValue(TEXT("EmissiveColor"), Spec.EmissiveColor);
		MID->SetScalarParameterValue(TEXT("EmissiveIntensity"), Spec.EmissiveIntensity);
		return MID;
	}

	void ApplySectionSpec(
		UProceduralMeshComponent* Mesh,
		int32 SectionIndex,
		const FlappyNeonPalette::FNeonSpec& Spec)
	{
		if (!Mesh)
		{
			return;
		}
		if (UMaterialInstanceDynamic* MID = MakeMID(Mesh, Spec))
		{
			Mesh->SetMaterial(SectionIndex, MID);
		}
	}
}
