package com.wizneylabs.agility

import android.app.Activity
import android.os.Handler
import android.os.Looper
import android.widget.FrameLayout
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.platform.ViewCompositionStrategy
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.LifecycleRegistry
import androidx.lifecycle.ViewModelStore
import androidx.lifecycle.ViewModelStoreOwner
import androidx.lifecycle.setViewTreeLifecycleOwner
import androidx.lifecycle.setViewTreeViewModelStoreOwner
import androidx.savedstate.SavedStateRegistry
import androidx.savedstate.SavedStateRegistryController
import androidx.savedstate.SavedStateRegistryOwner
import androidx.savedstate.setViewTreeSavedStateRegistryOwner

/**
 * Hosts the Jetpack Compose overlay on top of Unreal's GameActivity surface.
 *
 * GameActivity extends NativeActivity, which does NOT implement the AndroidX view-tree owners
 * (LifecycleOwner / ViewModelStoreOwner / SavedStateRegistryOwner) that ComposeView requires.
 * So this object carries its own implementations and feeds them the activity lifecycle that the
 * UPL injects into GameActivity. Without this, ComposeView throws on attach.
 *
 * The counter is owned by C++ (see AgilityComposeOverlay.cpp): button taps call nativeOnCounterDelta,
 * C++ mutates the value and calls pushCounter back, which drives recomposition.
 */
object AgilityComposeHost : LifecycleOwner, ViewModelStoreOwner, SavedStateRegistryOwner {

    private val lifecycleRegistry = LifecycleRegistry(this)
    private val store = ViewModelStore()
    private val savedStateRegistryController = SavedStateRegistryController.create(this)
    private val mainHandler = Handler(Looper.getMainLooper())

    private var attached = false

    /** The value Compose renders. Source of truth lives in C++; this mirrors it. */
    var counter by mutableIntStateOf(0)
        private set

    override val lifecycle: Lifecycle
        get() = lifecycleRegistry

    override val viewModelStore: ViewModelStore
        get() = store

    override val savedStateRegistry: SavedStateRegistry
        get() = savedStateRegistryController.savedStateRegistry

    /** Called from GameActivity.onCreate (injected via UPL). Must run on the UI thread. */
    @JvmStatic
    fun attach(activity: Activity) {
        if (attached) return
        attached = true

        savedStateRegistryController.performAttach()
        savedStateRegistryController.performRestore(null)
        lifecycleRegistry.currentState = Lifecycle.State.CREATED

        val composeView = ComposeView(activity).apply {
            setViewTreeLifecycleOwner(this@AgilityComposeHost)
            setViewTreeViewModelStoreOwner(this@AgilityComposeHost)
            setViewTreeSavedStateRegistryOwner(this@AgilityComposeHost)
            setViewCompositionStrategy(ViewCompositionStrategy.DisposeOnViewTreeLifecycleDestroyed)
            setContent {
                AgilityOverlayRoot(
                    counter = counter,
                    onDelta = { delta -> nativeOnCounterDelta(delta) },
                )
            }
        }

        // Overlay on top of Unreal's surface. The view's background stays transparent so the 3D
        // scene shows through everywhere the Compose content doesn't paint.
        val params = FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT,
        )
        activity.addContentView(composeView, params)
    }

    /** Called from C++ over JNI when the UE-owned counter changes. Marshals onto the UI thread. */
    @JvmStatic
    fun pushCounter(value: Int) {
        mainHandler.post { counter = value }
    }

    /** Compose -> C++. Bound to the extern "C" symbol in AgilityComposeOverlay.cpp. */
    @JvmStatic
    external fun nativeOnCounterDelta(delta: Int)

    @JvmStatic
    fun onStart() {
        lifecycleRegistry.currentState = Lifecycle.State.STARTED
    }

    @JvmStatic
    fun onResume() {
        lifecycleRegistry.currentState = Lifecycle.State.RESUMED
    }

    @JvmStatic
    fun onPause() {
        lifecycleRegistry.currentState = Lifecycle.State.STARTED
    }

    @JvmStatic
    fun onStop() {
        lifecycleRegistry.currentState = Lifecycle.State.CREATED
    }

    @JvmStatic
    fun onDestroy() {
        if (!attached) return
        lifecycleRegistry.currentState = Lifecycle.State.DESTROYED
        store.clear()
        attached = false
    }
}
