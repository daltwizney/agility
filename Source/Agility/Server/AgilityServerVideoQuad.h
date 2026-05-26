#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HttpFwd.h"
#include "AgilityServerVideoQuad.generated.h"

class FSocket;
class FUdpSocketReceiver;
class IImageWrapper;
class UMaterialInstanceDynamic;
class UProceduralMeshComponent;
class UTexture2D;

// Displays a UDP-streamed video on a procedurally generated quad. Pairs with
// the Python web server in python/server/ — on BeginPlay this actor:
//   1. POSTs /video/restart on the HTTP control endpoint (default
//      127.0.0.1:8000). The response carries the post-resize frame width /
//      height, which we use to size the quad and pre-allocate the dynamic
//      texture before any UDP frame lands.
//   2. Binds a UDP listener (default 127.0.0.1:8001) and decodes each
//      received datagram as a JPEG frame, pushing the pixels into the
//      dynamic texture bound to the same M_AgilityVideoUnlit MID that
//      AAgilityVideoQuad uses.
//
// Quad layout, base-material binding, and EndPlay teardown mirror
// AAgilityVideoQuad — only the pixel source differs (UDP/JPEG vs UMediaPlayer).
UCLASS()
class AGILITY_API AAgilityServerVideoQuad : public AActor
{
	GENERATED_BODY()

public:
	AAgilityServerVideoQuad();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "Agility|Server")
	FString ControlServerUrl = TEXT("http://127.0.0.1:8000");

	UPROPERTY(EditAnywhere, Category = "Agility|Server", meta = (ClampMin = "1", ClampMax = "65535"))
	int32 UdpListenPort = 8001;

	UPROPERTY(EditAnywhere, Category = "Agility|Server|Quad", meta = (ClampMin = "1.0"))
	float QuadHeight = 200.0f;

	UPROPERTY(VisibleAnywhere, Category = "Agility|Server")
	TObjectPtr<UProceduralMeshComponent> Mesh;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> FrameTexture;

	// Cached after /video/restart returns — also the size of FrameTexture.
	int32 FrameWidth = 0;
	int32 FrameHeight = 0;

	// Reassembly state for chunked JPEG frames. Each UDP datagram carries an
	// 8-byte header (frame_id u32, chunk_index u16, chunk_count u16) followed
	// by up to 8 KiB of JPEG payload. Chunks for a given frame_id are
	// accumulated into slots; when all slots are filled we concatenate and
	// hand to the JPEG decoder. Older frame_ids in flight when a new one
	// starts are discarded — newer frame supersedes.
	uint32 CurrentFrameId = 0;
	bool bHasCurrentFrame = false;
	int32 CurrentChunksReceived = 0;
	TArray<TArray<uint8>> CurrentChunks;

	FSocket* Socket = nullptr;
	TUniquePtr<FUdpSocketReceiver> Receiver;
	TSharedPtr<IImageWrapper> JpegDecoder;

	void BuildQuad(float AspectRatio);
	void ApplyBaseMaterial();

	void RequestRestart();
	void OnRestartResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);

	void StartUdpListener();
	void StopUdpListener();
	void HandleDatagram(const TArray<uint8>& Bytes);
	void DecodeAndUpload(const TArray<uint8>& JpegBytes);

	void EnsureTexture();
	void UploadFrameToTexture(const TArray64<uint8>& BgraPixels);
};
