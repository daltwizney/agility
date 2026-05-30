package com.wizneylabs.agility

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.statusBars
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

/**
 * Phase 1 dummy overlay, built from the Agility design system (see AgilityComposeTheme.kt). Pinned to
 * the TOP of the screen so it stays clear of the on-screen camera controls along the bottom. The +/-
 * buttons round-trip through C++: onDelta -> JNI -> UE mutates the actor counter -> pushCounter ->
 * recomposition.
 *
 * Note: this fills the screen so the panel can be top-aligned, so the overlay swallows touches across
 * the whole window. Touch pass-through is a Phase 2 concern (see compose-android/00-scope.md).
 */
@Composable
fun AgilityOverlayRoot(counter: Int, onDelta: (Int) -> Unit) {
    Box(
        modifier = Modifier
            .fillMaxSize()
            .windowInsetsPadding(WindowInsets.statusBars),
        contentAlignment = Alignment.TopCenter,
    ) {
        AgilityPanel(modifier = Modifier.padding(top = 16.dp)) {
            AgilityPanelHeader(title = "// AGILITY SYS", subtitle = "UNREAL × COMPOSE")
            Spacer(Modifier.height(14.dp))
            AgilityDivider()
            Spacer(Modifier.height(12.dp))
            Text(
                text = counter.toString(),
                color = AgilityTheme.colors.neon,
                style = AgilityTheme.type.display,
            )
            Spacer(Modifier.height(16.dp))
            Row(horizontalArrangement = Arrangement.spacedBy(16.dp)) {
                AgilityButton(label = "−", onClick = { onDelta(-1) })
                AgilityButton(label = "+", onClick = { onDelta(1) })
            }
        }
    }
}
