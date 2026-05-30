#pragma once

#include "CoreMinimal.h"

class AAgilityComposeCounterActor;

DECLARE_LOG_CATEGORY_EXTERN(LogAgilityCompose, Log, All);

/**
 * C++ side of the Jetpack Compose overlay bridge (Android only).
 *
 * The counter is owned by AAgilityComposeCounterActor. The Kotlin host (AgilityComposeHost.kt) calls
 * nativeOnCounterDelta (defined in the .cpp) when an overlay button is tapped; that call arrives on
 * the Android UI thread and is marshalled onto the game thread before touching the actor. The actor
 * then calls PushCounter to send its authoritative value back to Compose.
 *
 * On every non-Android platform these are no-ops, so editor / PIE / macOS builds are unaffected.
 */
namespace AgilityCompose
{
	/** The active actor the overlay drives. Last registration wins. */
	void RegisterCounterActor(AAgilityComposeCounterActor* Actor);
	void UnregisterCounterActor(AAgilityComposeCounterActor* Actor);

	/** Push the actor-owned counter value to the Compose overlay. Call from the game thread. */
	void PushCounter(int32 Value);
}
