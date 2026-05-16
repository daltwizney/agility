#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AgilityVideoQuad.generated.h"

class UFileMediaSource;
class UMaterialInstanceDynamic;
class UMediaPlayer;
class UMediaSoundComponent;
class UMediaTexture;
class UProceduralMeshComponent;

UCLASS()
class AGILITY_API AAgilityVideoQuad : public AActor
{
	GENERATED_BODY()

public:
	AAgilityVideoQuad();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// File name (no path) of a video under <Plugin>/Content/Movies/. Defaults to the bundled demo clip.
	UPROPERTY(EditAnywhere, Category = "Video")
	FString VideoFileName = TEXT("collie_agility.mp4");

	// World-space height of the quad in Unreal units (cm). Width is derived from AspectRatio.
	UPROPERTY(EditAnywhere, Category = "Video|Size", meta = (ClampMin = "1.0"))
	float QuadHeight = 200.0f;

	// Width / Height of the source video. Default matches collie_agility.mp4 (784 / 1168).
	UPROPERTY(EditAnywhere, Category = "Video|Size", meta = (ClampMin = "0.01"))
	float AspectRatio = 0.67123288f;

	UPROPERTY(EditAnywhere, Category = "Video|Playback")
	bool bLoop = true;

	UPROPERTY(EditAnywhere, Category = "Video|Playback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Volume = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Video")
	TObjectPtr<UProceduralMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Video")
	TObjectPtr<UMediaSoundComponent> MediaSound;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> MediaPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> MediaTexture;

	UPROPERTY(Transient)
	TObjectPtr<UFileMediaSource> MediaSource;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;

	void BuildQuad();
	void ApplyBaseMaterial();
	FString ResolveVideoPath() const;
};
