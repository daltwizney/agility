using UnrealBuildTool;

public class Agility : ModuleRules
{
	public Agility(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "ProceduralMeshComponent" });

		// Projects: IPluginManager (locate our own plugin dir at runtime).
		// RenderCore: AddShaderSourceDirectoryMapping (mount Shaders/ as /Plugin/Agility).
		PrivateDependencyModuleNames.AddRange(new string[] { "Projects", "RenderCore" });
	}
}
