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
		PrivateDependencyModuleNames.AddRange(new string[] { "Projects", "RenderCore", "MediaAssets", "AudioMixer", "HTTP", "Json" });

		// Stage raw video files (Content/Movies/*.mp4) into packaged builds. UE auto-stages .uasset content,
		// but raw files in a plugin's Content/Movies/ need to be declared explicitly.
		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Content", "Movies", "*.mp4"), StagedFileType.NonUFS);
	}
}
