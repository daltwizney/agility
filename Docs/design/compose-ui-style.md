# Agility Compose UI Style

The standard look for Compose UI overlaid on the Unreal surface on Android: a **sci-fi HUD** —
translucent dark-teal panels, neon-cyan accents, monospace type, asymmetric cut corners, and a soft
glow on hero numerals. Think console readout floating over the live 3D scene.

The style lives as a small **Compose design system**, not a style guide you eyeball and re-type. Build
new overlays from its primitives so the look stays consistent for free — and **devs can override it**:
it's the default, not a mandate.

## Where it lives

`Source/Agility/Android/java/com/wizneylabs/agility/AgilityComposeTheme.kt`

- **Tokens** — `AgilityTheme.colors`, `AgilityTheme.shapes`, `AgilityTheme.type`. Read from these
  instead of hard-coding hex values, `dp` corner sizes, or `TextStyle`s.
- **Primitives** — `AgilityPanel`, `AgilityPanelHeader`, `AgilityDivider`, `AgilityButton`. Compose
  your screen out of these rather than re-deriving panel borders / button press states each time.

`AgilityComposeUI.kt` (the Phase 1 counter overlay) is the reference consumer — a good template for a
new overlay.

## Usage

```kotlin
AgilityPanel(modifier = Modifier.padding(top = 16.dp)) {
    AgilityPanelHeader(title = "// AGILITY SYS", subtitle = "UNREAL × COMPOSE")
    Spacer(Modifier.height(14.dp))
    AgilityDivider()
    Spacer(Modifier.height(12.dp))
    Text("4", color = AgilityTheme.colors.Neon, style = AgilityTheme.type.Display)
    Spacer(Modifier.height(16.dp))
    Row(horizontalArrangement = Arrangement.spacedBy(16.dp)) {
        AgilityButton(label = "−", onClick = { /* ... */ })
        AgilityButton(label = "+", onClick = { /* ... */ })
    }
}
```

## Overriding the style

The design system is the default, not a lock-in. Tokens live in CompositionLocals (just like
`MaterialTheme`), so a dev restyles a subtree by wrapping it in the `AgilityTheme` provider and passing
an overridden token bundle. You only pass what you want to change; the rest inherits.

```kotlin
AgilityTheme(colors = AgilityColors(neon = Color(0xFFFF3B30))) {
    MyOverlay()   // every AgilityPanel / AgilityButton / etc. below now reads red
}
```

Because the primitives read everything through `AgilityTheme.*`, an override flows everywhere
automatically — you don't touch the composables. Derived tokens default off `neon` (e.g. `divider`,
`pressedFill`), so overriding just the accent carries through. Note: `type.display`'s glow colour is
baked off the default accent; if you re-accent and want the glow to match, also override `display`.

A dev can equally ignore the system entirely and write raw Compose — nothing forces these primitives.

## Conventions

- **Pin HUD panels to the top** (`Alignment.TopCenter` + `windowInsetsPadding(WindowInsets.statusBars)`).
  The bottom of the screen is where UE draws the on-screen camera/touch controls; a bottom-anchored
  panel covers them.
- **Keep panel fills translucent** so the 3D scene reads through behind the UI — that "floating HUD over
  the game" effect is the whole point. `AgilityColors.PanelFill` is already translucent; don't drop a
  fully opaque background over it.
- **Monospace everywhere.** It's load-bearing for the console aesthetic — `AgilityTheme.type` styles are
  all `FontFamily.Monospace`.
- **Add new tokens to the theme, not to the screen.** If a new overlay needs a colour/shape/type that
  isn't there yet, add it to `AgilityColors` / `AgilityShapes` / `AgilityType` so the next screen inherits
  it, rather than introducing a one-off literal in the composable.

## Why CompositionLocal-backed (not a static object)

`AgilityTheme` mirrors `MaterialTheme`: an accessor object whose `colors` / `shapes` / `type` read from
`staticCompositionLocalOf` defaults, plus a same-named provider composable to override them. Static
locals (not regular ones) because the theme rarely changes — no need to track reads. This is what makes
the "default, but overridable" requirement work without consumers changing: the defaults apply with no
provider in scope, and an `AgilityTheme { }` wrapper swaps them for a subtree.

## Android-only

This is Android-only by design, like the rest of the Compose overlay (see
[`../tutorials/compose-android/00-scope.md`](../tutorials/compose-android/00-scope.md)). The theme file
only compiles into the Android build via the UPL Kotlin copy step; nothing here runs in PIE or on macOS.
