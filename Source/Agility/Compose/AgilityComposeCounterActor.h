#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AgilityComposeCounterActor.generated.h"

class UStaticMeshComponent;

// A plain UE actor whose Counter state is driven from the Jetpack Compose overlay on Android.
//
// The overlay's +/- buttons call into C++ over JNI; the bridge routes the delta here on the game
// thread. The actor updates Counter, rotates its mesh so the change is visible in the 3D scene, and
// pushes the new value back to the overlay. Spawned automatically by UAgilityComposeWorldSubsystem,
// so no editor placement or game-mode wiring is needed. Off Android nothing drives it (the overlay
// doesn't exist), so it just sits there inert.
UCLASS()
class AGILITY_API AAgilityComposeCounterActor : public AActor
{
	GENERATED_BODY()

public:
	AAgilityComposeCounterActor();

	// Apply a delta originating from the Compose overlay. Game thread only.
	void ApplyDelta(int32 Delta);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Agility|Compose")
	int32 Counter = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agility|Compose")
	float DegreesPerStep = 15.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ApplyVisual();

	UPROPERTY(VisibleAnywhere, Category = "Agility|Compose")
	TObjectPtr<UStaticMeshComponent> Mesh;
};
