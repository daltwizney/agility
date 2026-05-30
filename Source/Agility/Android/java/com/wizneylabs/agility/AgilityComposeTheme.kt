package com.wizneylabs.agility

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.interaction.collectIsPressedAsState
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CutCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.ReadOnlyComposable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.runtime.staticCompositionLocalOf
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Shadow
import androidx.compose.ui.graphics.Shape
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

/**
 * Agility's sci-fi HUD design system — the default look for Compose UI overlaid on the Unreal surface
 * on Android. Translucent dark-teal panels, neon-cyan accents, monospace type, cut corners, and a soft
 * glow on hero numerals.
 *
 * It works like [androidx.compose.material3.MaterialTheme]: tokens live in CompositionLocals with
 * sensible defaults, so overlays just read [AgilityTheme.colors] / [AgilityTheme.shapes] /
 * [AgilityTheme.type] with no setup. To restyle a subtree, wrap it in the [AgilityTheme] provider and
 * pass an overridden token bundle:
 *
 * ```kotlin
 * AgilityTheme(colors = AgilityColors(neon = Color(0xFFFF3B30))) {
 *     MyOverlay()
 * }
 * ```
 *
 * Build new overlays from the primitives (AgilityPanel / AgilityButton / AgilityDivider /
 * AgilityPanelHeader) and read tokens through AgilityTheme rather than hard-coding hex values or sizes,
 * so an override flows everywhere automatically. See Docs/design/compose-ui-style.md for the rationale.
 */
object AgilityTheme {
    val colors: AgilityColors
        @Composable @ReadOnlyComposable get() = LocalAgilityColors.current

    val shapes: AgilityShapes
        @Composable @ReadOnlyComposable get() = LocalAgilityShapes.current

    val type: AgilityTypography
        @Composable @ReadOnlyComposable get() = LocalAgilityTypography.current
}

/**
 * Provides an overridden token bundle to [content]. Unspecified bundles inherit the values already in
 * scope (the built-in defaults at the root), so you only pass what you want to change.
 */
@Composable
fun AgilityTheme(
    colors: AgilityColors = AgilityTheme.colors,
    shapes: AgilityShapes = AgilityTheme.shapes,
    typography: AgilityTypography = AgilityTheme.type,
    content: @Composable () -> Unit,
) {
    CompositionLocalProvider(
        LocalAgilityColors provides colors,
        LocalAgilityShapes provides shapes,
        LocalAgilityTypography provides typography,
        content = content,
    )
}

/**
 * Colour tokens. Derived tokens (divider, pressedFill) default off [neon], so overriding just [neon]
 * carries the whole accent through.
 */
class AgilityColors(
    /** Primary accent: borders, hero numerals, button outlines and text. */
    val neon: Color = Color(0xFF00E5FF),
    /** Muted accent for secondary captions / subtitles. */
    val neonDim: Color = Color(0xFF4A8B95),
    /** Translucent panel background — lets the 3D scene read through behind the HUD. */
    val panelFill: Color = Color(0xCC061318),
    /** Hairline divider tint. */
    val divider: Color = neon.copy(alpha = 0.5f),
    /** Fill flashed under an AgilityButton while pressed. */
    val pressedFill: Color = neon.copy(alpha = 0.20f),
)

class AgilityShapes(
    /** Asymmetric cut corners give panels their "console readout" silhouette. */
    val panel: Shape = CutCornerShape(topStart = 18.dp, bottomEnd = 18.dp),
    /** Symmetric cut corners for interactive controls. */
    val button: Shape = CutCornerShape(8.dp),
)

/**
 * Type tokens — all monospace, which is load-bearing for the console aesthetic. [display]'s glow is
 * baked off the default accent; if you override [AgilityColors.neon] and want the glow to match,
 * override [display] too.
 */
class AgilityTypography(
    /** Section / system label, e.g. "// AGILITY SYS". */
    val label: TextStyle = TextStyle(
        fontFamily = FontFamily.Monospace,
        fontWeight = FontWeight.Bold,
        fontSize = 13.sp,
        letterSpacing = 4.sp,
    ),
    /** Dim caption under a label. */
    val caption: TextStyle = TextStyle(
        fontFamily = FontFamily.Monospace,
        fontSize = 10.sp,
        letterSpacing = 2.sp,
    ),
    /** Hero numeral / readout, glowing. */
    val display: TextStyle = TextStyle(
        fontFamily = FontFamily.Monospace,
        fontWeight = FontWeight.Bold,
        fontSize = 68.sp,
        letterSpacing = 6.sp,
        shadow = Shadow(color = Color(0xFF00E5FF).copy(alpha = 0.7f), blurRadius = 28f),
    ),
    /** Button glyph / label. */
    val button: TextStyle = TextStyle(
        fontFamily = FontFamily.Monospace,
        fontWeight = FontWeight.Bold,
        fontSize = 28.sp,
    ),
)

// Theme rarely changes, so static locals avoid tracking reads. Defaults give the built-in look with
// no provider in scope.
val LocalAgilityColors = staticCompositionLocalOf { AgilityColors() }
val LocalAgilityShapes = staticCompositionLocalOf { AgilityShapes() }
val LocalAgilityTypography = staticCompositionLocalOf { AgilityTypography() }

/**
 * Standard HUD panel: clipped + filled + neon-bordered with the Agility cut-corner silhouette.
 * Lay out panel contents in the [content] column.
 */
@Composable
fun AgilityPanel(
    modifier: Modifier = Modifier,
    content: @Composable ColumnScope.() -> Unit,
) {
    Column(
        modifier = modifier
            .clip(AgilityTheme.shapes.panel)
            .background(AgilityTheme.colors.panelFill)
            .border(1.dp, AgilityTheme.colors.neon, AgilityTheme.shapes.panel)
            .padding(horizontal = 28.dp, vertical = 18.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        content = content,
    )
}

/** Title + optional subtitle, centered, in the standard label/caption type. */
@Composable
fun AgilityPanelHeader(title: String, subtitle: String? = null) {
    Text(text = title, color = AgilityTheme.colors.neon, style = AgilityTheme.type.label)
    if (subtitle != null) {
        Spacer(Modifier.height(4.dp))
        Text(text = subtitle, color = AgilityTheme.colors.neonDim, style = AgilityTheme.type.caption)
    }
}

/** Hairline neon divider. */
@Composable
fun AgilityDivider(width: Dp = 200.dp) {
    Box(
        modifier = Modifier
            .width(width)
            .height(1.dp)
            .background(AgilityTheme.colors.divider),
    )
}

/** Standard interactive control: cut-corner outline, neon glyph, fill flash on press. */
@Composable
fun AgilityButton(
    label: String,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val interaction = remember { MutableInteractionSource() }
    val pressed by interaction.collectIsPressedAsState()
    Box(
        modifier = modifier
            .size(width = 88.dp, height = 56.dp)
            .clip(AgilityTheme.shapes.button)
            .background(if (pressed) AgilityTheme.colors.pressedFill else Color.Transparent)
            .border(1.5.dp, AgilityTheme.colors.neon, AgilityTheme.shapes.button)
            .clickable(interactionSource = interaction, indication = null) { onClick() },
        contentAlignment = Alignment.Center,
    ) {
        Text(text = label, color = AgilityTheme.colors.neon, style = AgilityTheme.type.button)
    }
}
