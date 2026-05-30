#include "AgilityComposeCounterActor.h"

#include "AgilityComposeOverlay.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AAgilityComposeCounterActor::AAgilityComposeCounterActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		Mesh->SetStaticMesh(CubeFinder.Object);
	}
}

void AAgilityComposeCounterActor::BeginPlay()
{
	Super::BeginPlay();

	AgilityCompose::RegisterCounterActor(this);
	ApplyVisual();
	AgilityCompose::PushCounter(Counter);
}

void AAgilityComposeCounterActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AgilityCompose::UnregisterCounterActor(this);
	Super::EndPlay(EndPlayReason);
}

void AAgilityComposeCounterActor::ApplyDelta(int32 Delta)
{
	Counter += Delta;
	UE_LOG(LogAgilityCompose, Log, TEXT("Actor counter -> %d"), Counter);

	ApplyVisual();
	AgilityCompose::PushCounter(Counter);
}

void AAgilityComposeCounterActor::ApplyVisual()
{
	// Make the state change visible in the 3D scene: yaw proportional to the counter.
	SetActorRelativeRotation(FRotator(0.f, Counter * DegreesPerStep, 0.f));
}
