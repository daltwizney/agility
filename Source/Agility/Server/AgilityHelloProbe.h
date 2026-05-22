#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HttpFwd.h"
#include "AgilityHelloProbe.generated.h"

class USceneComponent;

// Smallest-possible end-to-end client of the Python-side web server in
// python/server/. On BeginPlay, fires one HTTP GET at ServerUrl, parses the
// JSON `{ "message": "..." }` response, and posts the message to UE's
// on-screen debug overlay. Drag the actor into any map, start the server with
// `uv run uvicorn server.main:app --host 127.0.0.1 --app-dir .` from python/,
// hit Play — you should see "Server says: hello agility" in cyan top-left.
//
// Used as the sanity-check integration test before either side grows beyond
// a single endpoint. See python/server/README.md for the server side.
UCLASS()
class AGILITY_API AAgilityHelloProbe : public AActor
{
	GENERATED_BODY()

public:
	AAgilityHelloProbe();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Agility|Server")
	TObjectPtr<USceneComponent> Root;

	// Defaults to uvicorn's localhost dev bind (127.0.0.1:8000) so this works
	// out-of-the-box against `uv run uvicorn server.main:app` from python/.
	// Change in the Details panel to point at a different host / port.
	UPROPERTY(EditAnywhere, Category = "Agility|Server")
	FString ServerUrl = TEXT("http://127.0.0.1:8000/hello");

	UPROPERTY(EditAnywhere, Category = "Agility|Server", meta = (ClampMin = "1.0"))
	float MessageDuration = 30.0f;

private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
};
