#include "AgilityVideoQuad.h"

#include "FileMediaSource.h"
#include "MediaSoundComponent.h"
#include "Interfaces/IPluginManager.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/Paths.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// Created as a one-time editor step (see Docs / handoff instructions): an Unlit, Two-Sided
	// material with a Texture2D parameter named "Video" wired into Emissive Color.
	const TCHAR* GBaseMaterialPath = TEXT("/Agility/Materials/M_AgilityVideoUnlit.M_AgilityVideoUnlit");
	const FName GVideoParamName(TEXT("Video"));
}

AAgilityVideoQuad::AAgilityVideoQuad()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	MediaSound = CreateDefaultSubobject<UMediaSoundComponent>(TEXT("MediaSound"));
	MediaSound->SetupAttachment(Mesh);
}

void AAgilityVideoQuad::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildQuad();
	ApplyBaseMaterial();
}

void AAgilityVideoQuad::BeginPlay()
{
	Super::BeginPlay();

	const FString VideoPath = ResolveVideoPath();
	if (!FPaths::FileExists(VideoPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("AgilityVideoQuad: video file not found at %s"), *VideoPath);
		return;
	}

	// Recreate the MID at play time — Transient UPROPERTYs don't survive editor→PIE duplication reliably.
	ApplyBaseMaterial();

	MediaPlayer = NewObject<UMediaPlayer>(this, NAME_None, RF_Transient);
	MediaPlayer->SetLooping(bLoop);
	MediaPlayer->PlayOnOpen = true;

	MediaTexture = NewObject<UMediaTexture>(this, NAME_None, RF_Transient);
	MediaTexture->AutoClear = true;
	MediaTexture->SetMediaPlayer(MediaPlayer);
	MediaTexture->UpdateResource();

	MediaSource = NewObject<UFileMediaSource>(this, NAME_None, RF_Transient);
	MediaSource->SetFilePath(VideoPath);

	if (MaterialInstance)
	{
		MaterialInstance->SetTextureParameterValue(GVideoParamName, MediaTexture);
		UE_LOG(LogTemp, Log, TEXT("AgilityVideoQuad: bound MediaTexture to '%s' on MID."), *GVideoParamName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AgilityVideoQuad: MaterialInstance is null at BeginPlay — video texture cannot be bound. Check that %s exists."), GBaseMaterialPath);
	}

	if (MediaSound)
	{
		MediaSound->SetMediaPlayer(MediaPlayer);
		MediaSound->SetVolumeMultiplier(Volume);
	}

	MediaPlayer->OpenSource(MediaSource);
	UE_LOG(LogTemp, Log, TEXT("AgilityVideoQuad: opened %s (loop=%s, volume=%.2f)"), *VideoPath, bLoop ? TEXT("true") : TEXT("false"), Volume);
}

void AAgilityVideoQuad::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MediaPlayer)
	{
		MediaPlayer->Close();
	}
	Super::EndPlay(EndPlayReason);
}

void AAgilityVideoQuad::BuildQuad()
{
	if (!Mesh)
	{
		return;
	}

	// Portrait/landscape-agnostic quad in the YZ plane, centered on the actor's origin.
	// Surface faces +X; the material is two-sided so it reads correctly from either side.
	const float HalfW = 0.5f * QuadHeight * AspectRatio;
	const float HalfH = 0.5f * QuadHeight;

	const FVector A(0.0f, -HalfW, +HalfH); // top-left
	const FVector B(0.0f, +HalfW, +HalfH); // top-right
	const FVector C(0.0f, +HalfW, -HalfH); // bottom-right
	const FVector D(0.0f, -HalfW, -HalfH); // bottom-left

	TArray<FVector> Vertices = { A, B, C, D };
	TArray<int32> Triangles = { 0, 2, 1, 0, 3, 2 };
	TArray<FVector> Normals = { FVector::ForwardVector, FVector::ForwardVector, FVector::ForwardVector, FVector::ForwardVector };
	// Vertex A sits at world -Y, which is the viewer's RIGHT when looking at
	// the +X-facing front (UE is left-handed). Texture column 0 needs to land
	// at the viewer's left, so A gets UV (1, 0) and B (at +Y) gets UV (0, 0).
	TArray<FVector2D> UV0 = { FVector2D(1, 0), FVector2D(0, 0), FVector2D(0, 1), FVector2D(1, 1) };
	TArray<FProcMeshTangent> Tangents = {
		FProcMeshTangent(0.0f, 1.0f, 0.0f),
		FProcMeshTangent(0.0f, 1.0f, 0.0f),
		FProcMeshTangent(0.0f, 1.0f, 0.0f),
		FProcMeshTangent(0.0f, 1.0f, 0.0f),
	};
	TArray<FLinearColor> Colors;

	Mesh->ClearAllMeshSections();
	Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, Colors, Tangents, /*bCreateCollision=*/false);
}

void AAgilityVideoQuad::ApplyBaseMaterial()
{
	if (!Mesh)
	{
		return;
	}

	UMaterial* Base = LoadObject<UMaterial>(nullptr, GBaseMaterialPath);
	if (!Base)
	{
		UE_LOG(LogTemp, Warning, TEXT("AgilityVideoQuad: failed to load base material at %s — confirm the asset exists at this exact path inside the Agility plugin's Content/Materials folder."), GBaseMaterialPath);
		return;
	}

	MaterialInstance = UMaterialInstanceDynamic::Create(Base, this);
	Mesh->SetMaterial(0, MaterialInstance);
	UE_LOG(LogTemp, Log, TEXT("AgilityVideoQuad: applied MID derived from %s"), GBaseMaterialPath);
}

FString AAgilityVideoQuad::ResolveVideoPath() const
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("Agility"));
	if (!Plugin.IsValid())
	{
		return FString();
	}
	return FPaths::Combine(Plugin->GetBaseDir(), TEXT("Content"), TEXT("Movies"), VideoFileName);
}
