#include "AgilityComposeWorldSubsystem.h"

#include "AgilityComposeCounterActor.h"
#include "Engine/World.h"

void UAgilityComposeWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (!InWorld.IsGameWorld())
	{
		return;
	}

	InWorld.SpawnActor<AAgilityComposeCounterActor>();
}
