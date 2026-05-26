#include "AgilityServerVideoQuad.h"

#include "Async/Async.h"
#include "Common/UdpSocketBuilder.h"
#include "Common/UdpSocketReceiver.h"
#include "Dom/JsonObject.h"
#include "Engine/Texture2D.h"
#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Modules/ModuleManager.h"
#include "ProceduralMeshComponent.h"
#include "RHI.h"
#include "Serialization/ArrayReader.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogAgilityServerVideoQuad, Log, All);

namespace
{
	// Same base material AAgilityVideoQuad uses: Unlit, Two-Sided, with a
	// Texture2D parameter "Video" wired into Emissive Color. Names are
	// prefixed to avoid collisions inside the unity build's merged
	// anonymous namespace (AgilityVideoQuad.cpp uses the bare names).
	const TCHAR* GServerVideoBaseMaterialPath = TEXT("/Agility/Materials/M_AgilityVideoUnlit.M_AgilityVideoUnlit");
	const FName GServerVideoParamName(TEXT("Video"));
}

AAgilityServerVideoQuad::AAgilityServerVideoQuad()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

void AAgilityServerVideoQuad::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	// Editor preview: build a square placeholder until we learn the real ratio
	// from /video/restart at BeginPlay.
	BuildQuad(1.0f);
	ApplyBaseMaterial();
}

void AAgilityServerVideoQuad::BeginPlay()
{
	Super::BeginPlay();

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	JpegDecoder = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	ApplyBaseMaterial();
	RequestRestart();
}

void AAgilityServerVideoQuad::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopUdpListener();
	JpegDecoder.Reset();
	if (FrameTexture)
	{
		FrameTexture->RemoveFromRoot();
		FrameTexture = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void AAgilityServerVideoQuad::BuildQuad(float AspectRatio)
{
	if (!Mesh)
	{
		return;
	}

	// Quad in the YZ plane, centered on the actor's origin, surface facing +X.
	// Same convention as AAgilityVideoQuad so both display the same way.
	const float HalfW = 0.5f * QuadHeight * AspectRatio;
	const float HalfH = 0.5f * QuadHeight;

	const FVector A(0.0f, -HalfW, +HalfH);
	const FVector B(0.0f, +HalfW, +HalfH);
	const FVector C(0.0f, +HalfW, -HalfH);
	const FVector D(0.0f, -HalfW, -HalfH);

	TArray<FVector> Vertices = { A, B, C, D };
	TArray<int32> Triangles = { 0, 2, 1, 0, 3, 2 };
	TArray<FVector> Normals = { FVector::ForwardVector, FVector::ForwardVector, FVector::ForwardVector, FVector::ForwardVector };
	// U is flipped vs. AAgilityVideoQuad: vertex A sits at world -Y but that's
	// to the viewer's RIGHT when looking at the +X-facing front (UE is
	// left-handed). Texture column 0 needs to land at the viewer's left, so
	// vertex A gets UV (1, 0) and vertex B gets UV (0, 0).
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

void AAgilityServerVideoQuad::ApplyBaseMaterial()
{
	if (!Mesh)
	{
		return;
	}

	UMaterial* Base = LoadObject<UMaterial>(nullptr, GServerVideoBaseMaterialPath);
	if (!Base)
	{
		UE_LOG(LogAgilityServerVideoQuad, Warning, TEXT("ServerVideoQuad: failed to load base material at %s"), GServerVideoBaseMaterialPath);
		return;
	}

	MaterialInstance = UMaterialInstanceDynamic::Create(Base, this);
	Mesh->SetMaterial(0, MaterialInstance);
}

void AAgilityServerVideoQuad::RequestRestart()
{
	const FString Url = ControlServerUrl + TEXT("/video/restart");
	UE_LOG(LogAgilityServerVideoQuad, Log, TEXT("ServerVideoQuad: POST %s"), *Url);

	const TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(TEXT("{}"));
	Request->OnProcessRequestComplete().BindUObject(this, &AAgilityServerVideoQuad::OnRestartResponse);
	Request->ProcessRequest();
}

void AAgilityServerVideoQuad::OnRestartResponse(FHttpRequestPtr /*Request*/, FHttpResponsePtr Response, bool bSuccess)
{
	if (!bSuccess || !Response.IsValid() || Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300)
	{
		UE_LOG(LogAgilityServerVideoQuad, Warning,
			TEXT("ServerVideoQuad: /video/restart failed — is the python server running on %s?"), *ControlServerUrl);
		return;
	}

	const FString Body = Response->GetContentAsString();
	TSharedPtr<FJsonObject> Json;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		UE_LOG(LogAgilityServerVideoQuad, Warning, TEXT("ServerVideoQuad: /video/restart returned non-JSON: %s"), *Body);
		return;
	}

	int32 W = 0;
	int32 H = 0;
	if (!Json->TryGetNumberField(TEXT("width"), W) || !Json->TryGetNumberField(TEXT("height"), H) || W <= 0 || H <= 0)
	{
		UE_LOG(LogAgilityServerVideoQuad, Warning, TEXT("ServerVideoQuad: /video/restart missing width/height: %s"), *Body);
		return;
	}

	FrameWidth = W;
	FrameHeight = H;
	UE_LOG(LogAgilityServerVideoQuad, Log, TEXT("ServerVideoQuad: frame size %dx%d, binding UDP listener on %d"), W, H, UdpListenPort);

	BuildQuad(static_cast<float>(W) / static_cast<float>(H));
	EnsureTexture();
	StartUdpListener();
}

void AAgilityServerVideoQuad::EnsureTexture()
{
	if (FrameWidth <= 0 || FrameHeight <= 0)
	{
		return;
	}

	FrameTexture = UTexture2D::CreateTransient(FrameWidth, FrameHeight, PF_B8G8R8A8);
	FrameTexture->SRGB = true;
	FrameTexture->Filter = TF_Bilinear;
	FrameTexture->AddToRoot();    // keep alive across PIE GC sweeps; released in EndPlay
	FrameTexture->UpdateResource();

	if (MaterialInstance)
	{
		MaterialInstance->SetTextureParameterValue(GServerVideoParamName, FrameTexture);
	}
}

void AAgilityServerVideoQuad::StartUdpListener()
{
	StopUdpListener();

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		UE_LOG(LogAgilityServerVideoQuad, Warning, TEXT("ServerVideoQuad: no socket subsystem"));
		return;
	}

	// Bind to loopback so only the local python server (which sends from
	// 127.0.0.1) can reach us. macOS caps SO_RCVBUF around 768 KB by default;
	// requesting 1 MB just asks the OS for as much as it'll give us. UDP
	// drops are fine in principle — newer frame supersedes older — but with
	// chunked frames a single dropped chunk loses the whole frame, so we
	// want headroom against burst arrival.
	Socket = FUdpSocketBuilder(TEXT("AgilityServerVideoQuadSocket"))
		.AsNonBlocking()
		.AsReusable()
		.BoundToAddress(FIPv4Address(127, 0, 0, 1))
		.BoundToPort(UdpListenPort)
		.WithReceiveBufferSize(1024 * 1024);

	if (!Socket)
	{
		UE_LOG(LogAgilityServerVideoQuad, Warning, TEXT("ServerVideoQuad: failed to bind UDP port %d"), UdpListenPort);
		return;
	}

	const FTimespan WaitTime = FTimespan::FromMilliseconds(10);
	Receiver = MakeUnique<FUdpSocketReceiver>(Socket, WaitTime, TEXT("AgilityServerVideoQuadReceiver"));

	// Receiver thread context — capture a weak ptr so a frame arriving during
	// EndPlay teardown doesn't dereference a half-destroyed actor. The
	// receiver thread is joined before the actor is destroyed (StopUdpListener
	// in EndPlay calls Receiver.Reset() which joins), so member access through
	// `Self` is safe while this lambda runs.
	//
	// Chunk reassembly happens HERE on the receiver thread — we only AsyncTask
	// to the game thread once per *assembled frame* (24/sec) rather than per
	// *packet* (168/sec at 7 chunks per frame). Per-packet AsyncTask + UE_LOG
	// was saturating the receiver thread; the OS socket buffer was overflowing
	// and packets were being dropped, leaving every frame missing chunks.
	TWeakObjectPtr<AAgilityServerVideoQuad> WeakSelf(this);
	Receiver->OnDataReceived().BindLambda([WeakSelf](const FArrayReaderPtr& Data, const FIPv4Endpoint&)
	{
		AAgilityServerVideoQuad* Self = WeakSelf.Get();
		if (!Self || !Data.IsValid())
		{
			return;
		}
		Self->HandleDatagram(*Data);
	});

	Receiver->Start();
}

void AAgilityServerVideoQuad::StopUdpListener()
{
	// UDP-only cleanup. Texture lifecycle is intentionally NOT touched here —
	// StartUdpListener() calls this at the top to clear any prior socket state,
	// and used to also clear FrameTexture, which silently nuked the texture
	// EnsureTexture() had just created in OnRestartResponse. Texture is now
	// released in EndPlay() instead.
	if (Receiver)
	{
		Receiver->Stop();
		Receiver.Reset();
	}
	if (Socket)
	{
		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		if (SocketSubsystem)
		{
			Socket->Close();
			SocketSubsystem->DestroySocket(Socket);
		}
		Socket = nullptr;
	}
}

void AAgilityServerVideoQuad::HandleDatagram(const TArray<uint8>& Bytes)
{
	// Runs on the receiver thread. Reassembles chunks in-place into per-actor
	// state; when a frame's last chunk arrives, hops to the game thread for
	// the decode + texture upload (the only steps that touch UE resources).
	constexpr int32 HeaderBytes = 8;
	if (Bytes.Num() < HeaderBytes)
	{
		return;
	}

	const uint8* B = Bytes.GetData();
	const uint32 FrameId =
		(static_cast<uint32>(B[0]) << 24) | (static_cast<uint32>(B[1]) << 16) |
		(static_cast<uint32>(B[2]) << 8)  |  static_cast<uint32>(B[3]);
	const uint16 ChunkIndex = static_cast<uint16>((B[4] << 8) | B[5]);
	const uint16 ChunkCount = static_cast<uint16>((B[6] << 8) | B[7]);
	const int32 PayloadBytes = Bytes.Num() - HeaderBytes;

	if (ChunkCount == 0 || ChunkIndex >= ChunkCount)
	{
		return;
	}

	if (!bHasCurrentFrame || FrameId != CurrentFrameId)
	{
		CurrentFrameId = FrameId;
		bHasCurrentFrame = true;
		CurrentChunksReceived = 0;
		CurrentChunks.Reset();
		CurrentChunks.SetNum(ChunkCount);
	}

	if (CurrentChunks.Num() != ChunkCount)
	{
		bHasCurrentFrame = false;
		return;
	}

	if (CurrentChunks[ChunkIndex].Num() == 0 && PayloadBytes > 0)
	{
		CurrentChunks[ChunkIndex].Append(B + HeaderBytes, PayloadBytes);
		++CurrentChunksReceived;
	}

	if (CurrentChunksReceived < ChunkCount)
	{
		return;
	}

	int32 TotalBytes = 0;
	for (const TArray<uint8>& Slot : CurrentChunks) { TotalBytes += Slot.Num(); }
	TArray<uint8> Assembled;
	Assembled.Reserve(TotalBytes);
	for (const TArray<uint8>& Slot : CurrentChunks) { Assembled.Append(Slot); }

	bHasCurrentFrame = false;
	CurrentChunksReceived = 0;
	CurrentChunks.Reset();

	TWeakObjectPtr<AAgilityServerVideoQuad> WeakSelf(this);
	AsyncTask(ENamedThreads::GameThread, [WeakSelf, JpegBytes = MoveTemp(Assembled)]()
	{
		if (AAgilityServerVideoQuad* Self = WeakSelf.Get())
		{
			Self->DecodeAndUpload(JpegBytes);
		}
	});
}

void AAgilityServerVideoQuad::DecodeAndUpload(const TArray<uint8>& JpegBytes)
{
	if (!JpegDecoder.IsValid() || !FrameTexture)
	{
		return;
	}

	if (!JpegDecoder->SetCompressed(JpegBytes.GetData(), JpegBytes.Num()))
	{
		UE_LOG(LogAgilityServerVideoQuad, Warning, TEXT("ServerVideoQuad: SetCompressed failed for %d-byte JPEG"), JpegBytes.Num());
		return;
	}

	if (JpegDecoder->GetWidth() != FrameWidth || JpegDecoder->GetHeight() != FrameHeight)
	{
		UE_LOG(LogAgilityServerVideoQuad, Warning, TEXT("ServerVideoQuad: JPEG size %lldx%lld != expected %dx%d"),
			JpegDecoder->GetWidth(), JpegDecoder->GetHeight(), FrameWidth, FrameHeight);
		return;
	}

	// TArray64 avoids the TArray<uint8> overload of GetRaw that UE 5.7's
	// IImageWrapper header flags as "often broken" — that wrapper performs a
	// non-obvious allocator conversion that can silently emit garbage.
	TArray64<uint8> Raw;
	if (!JpegDecoder->GetRaw(ERGBFormat::BGRA, 8, Raw))
	{
		UE_LOG(LogAgilityServerVideoQuad, Warning, TEXT("ServerVideoQuad: GetRaw(BGRA) failed"));
		return;
	}

	if (Raw.Num() < static_cast<int64>(FrameWidth) * FrameHeight * 4)
	{
		return;
	}

	UploadFrameToTexture(Raw);
}

void AAgilityServerVideoQuad::UploadFrameToTexture(const TArray64<uint8>& BgraPixels)
{
	if (!FrameTexture)
	{
		return;
	}
	const int64 PixelBytes = static_cast<int64>(FrameWidth) * FrameHeight * 4;
	if (BgraPixels.Num() < PixelBytes)
	{
		return;
	}

	// UpdateTextureRegions takes ownership of the source buffer + region (frees
	// them in the cleanup lambda once the render thread is done) so both must
	// be heap-allocated and outlive this function call.
	uint8* Buffer = static_cast<uint8*>(FMemory::Malloc(PixelBytes));
	FMemory::Memcpy(Buffer, BgraPixels.GetData(), PixelBytes);

	FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(0, 0, 0, 0, FrameWidth, FrameHeight);

	FrameTexture->UpdateTextureRegions(
		/*MipIndex=*/0, /*NumRegions=*/1, Region,
		/*SrcPitch=*/FrameWidth * 4, /*SrcBpp=*/4, Buffer,
		[](uint8* Data, const FUpdateTextureRegion2D* Regions) { FMemory::Free(Data); delete Regions; });
}
