#include "AgilityHelloProbe.h"

#include "Components/SceneComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/Engine.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogAgilityHelloProbe, Log, All);

AAgilityHelloProbe::AAgilityHelloProbe()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void AAgilityHelloProbe::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogAgilityHelloProbe, Log, TEXT("AgilityHelloProbe: GET %s"), *ServerUrl);

	const TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(ServerUrl);
	Request->SetVerb(TEXT("GET"));
	// BindUObject auto-tracks `this` lifetime — if the actor is destroyed
	// before the response lands, the callback is silently skipped.
	Request->OnProcessRequestComplete().BindUObject(this, &AAgilityHelloProbe::OnResponseReceived);
	Request->ProcessRequest();
}

void AAgilityHelloProbe::OnResponseReceived(FHttpRequestPtr /*Request*/, FHttpResponsePtr Response, bool bSuccess)
{
	auto ShowMessage = [this](const FColor& Color, const FString& Text)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, MessageDuration, Color, Text);
		}
	};

	if (!bSuccess || !Response.IsValid())
	{
		const FString ErrText = FString::Printf(
			TEXT("Agility server unreachable at %s — is `uv run uvicorn server.main:app --host 127.0.0.1 --app-dir .` running in python/?"),
			*ServerUrl);
		UE_LOG(LogAgilityHelloProbe, Warning, TEXT("%s"), *ErrText);
		ShowMessage(FColor::Red, ErrText);
		return;
	}

	const int32 Code = Response->GetResponseCode();
	if (Code < 200 || Code >= 300)
	{
		const FString ErrText = FString::Printf(TEXT("Agility server returned HTTP %d at %s"), Code, *ServerUrl);
		UE_LOG(LogAgilityHelloProbe, Warning, TEXT("%s"), *ErrText);
		ShowMessage(FColor::Red, ErrText);
		return;
	}

	const FString Body = Response->GetContentAsString();

	TSharedPtr<FJsonObject> Json;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		const FString ErrText = FString::Printf(TEXT("Agility server returned non-JSON: %s"), *Body);
		UE_LOG(LogAgilityHelloProbe, Warning, TEXT("%s"), *ErrText);
		ShowMessage(FColor::Red, ErrText);
		return;
	}

	FString Message;
	if (!Json->TryGetStringField(TEXT("message"), Message))
	{
		const FString ErrText = FString::Printf(TEXT("Agility server response missing 'message' field: %s"), *Body);
		UE_LOG(LogAgilityHelloProbe, Warning, TEXT("%s"), *ErrText);
		ShowMessage(FColor::Red, ErrText);
		return;
	}

	const FString OkText = FString::Printf(TEXT("Server says: %s"), *Message);
	UE_LOG(LogAgilityHelloProbe, Log, TEXT("%s"), *OkText);
	ShowMessage(FColor::Cyan, OkText);
}
