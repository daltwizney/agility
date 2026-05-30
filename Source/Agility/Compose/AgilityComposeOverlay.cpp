#include "AgilityComposeOverlay.h"

DEFINE_LOG_CATEGORY(LogAgilityCompose);

#if PLATFORM_ANDROID

#include "AgilityComposeCounterActor.h"
#include "Android/AndroidApplication.h"
#include "Async/Async.h"
#include <jni.h>

namespace
{
	TWeakObjectPtr<AAgilityComposeCounterActor> GCounterActor;

	// Resolved lazily from the jclass JNI hands us on the first button tap — that class reference comes
	// from the app's classloader, which sidesteps the FindClass-from-a-native-thread classloader trap.
	jclass GHostClass = nullptr;
	jmethodID GPushCounterMethod = nullptr;
}

// Compose -> C++. Bound by name to AgilityComposeHost.nativeOnCounterDelta. The mangled symbol must
// match the Kotlin package + class exactly (com.wizneylabs.agility.AgilityComposeHost).
extern "C" JNIEXPORT void JNICALL
Java_com_wizneylabs_agility_AgilityComposeHost_nativeOnCounterDelta(JNIEnv* Env, jclass HostClass, jint Delta)
{
	// Android UI thread. Cache the push-back binding here, where the app classloader context is valid.
	if (GPushCounterMethod == nullptr)
	{
		GHostClass = static_cast<jclass>(Env->NewGlobalRef(HostClass));
		GPushCounterMethod = Env->GetStaticMethodID(GHostClass, "pushCounter", "(I)V");
	}

	const int32 DeltaValue = static_cast<int32>(Delta);

	// AActor / UObject access is game-thread only — hop off the UI thread before touching the actor.
	AsyncTask(ENamedThreads::GameThread, [DeltaValue]()
	{
		if (AAgilityComposeCounterActor* Actor = GCounterActor.Get())
		{
			Actor->ApplyDelta(DeltaValue);
		}
		else
		{
			UE_LOG(LogAgilityCompose, Warning, TEXT("Counter delta %d ignored — no counter actor registered"), DeltaValue);
		}
	});
}

namespace AgilityCompose
{
	void RegisterCounterActor(AAgilityComposeCounterActor* Actor)
	{
		GCounterActor = Actor;
	}

	void UnregisterCounterActor(AAgilityComposeCounterActor* Actor)
	{
		if (GCounterActor.Get() == Actor)
		{
			GCounterActor.Reset();
		}
	}

	void PushCounter(int32 Value)
	{
		if (GHostClass != nullptr && GPushCounterMethod != nullptr)
		{
			if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
			{
				Env->CallStaticVoidMethod(GHostClass, GPushCounterMethod, static_cast<jint>(Value));
			}
		}
	}
}

#else // !PLATFORM_ANDROID

namespace AgilityCompose
{
	void RegisterCounterActor(AAgilityComposeCounterActor*) {}
	void UnregisterCounterActor(AAgilityComposeCounterActor*) {}
	void PushCounter(int32) {}
}

#endif
