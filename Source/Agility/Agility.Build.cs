using System.IO;
using UnrealBuildTool;

public class Agility : ModuleRules
{
	public Agility(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "ProceduralMeshComponent" });

		// Projects: IPluginManager (locate our own plugin dir at runtime).
		// RenderCore: AddShaderSourceDirectoryMapping (mount Shaders/ as /Plugin/Agility).
		// MediaAssets: UMediaPlayer / UMediaTexture / UFileMediaSource / UMediaSoundComponent for the video quad actor.
		// AudioMixer: USynthComponent (UMediaSoundComponent's parent) lives here — SetVolumeMultiplier needs it linked.
		// HTTP: FHttpModule + IHttpRequest for AAgilityHelloProbe (and future python/server/ clients).
		// Json: FJsonObject + TJsonReader for parsing server responses.
		// Sockets + Networking: FUdpSocketBuilder + FUdpSocketReceiver for AAgilityServerVideoQuad's UDP listener.
		// ImageWrapper: JPEG decode for UDP-received video frames.
		PrivateDependencyModuleNames.AddRange(new string[] { "Projects", "RenderCore", "MediaAssets", "AudioMixer", "HTTP", "Json", "Sockets", "Networking", "ImageWrapper" });

		// Stage raw video files (Content/Movies/*.mp4) into packaged builds. UE auto-stages .uasset content,
		// but raw files in a plugin's Content/Movies/ need to be declared explicitly.
		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Content", "Movies", "*.mp4"), StagedFileType.NonUFS);

		// Android-only: attach the UPL that wires Kotlin + Jetpack Compose into UE's generated gradle build
		// and overlays a ComposeView on GameActivity's surface. See Source/Agility/Compose/AgilityComposeOverlay.cpp
		// for the C++ side of the JNI bridge (which compiles to no-ops on every other platform).
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "Android", "Agility_UPL_Android.xml"));

			// FAndroidApplication::GetJavaEnv() for the JNI bridge in Compose/AgilityComposeOverlay.cpp.
			PrivateDependencyModuleNames.Add("ApplicationCore");
		}
	}
}
