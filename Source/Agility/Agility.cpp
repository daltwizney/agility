#include "Agility.h"

#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "ShaderCore.h"

class FAgilityModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		// Mount the plugin's Shaders/ directory under a virtual path so HLSL files inside
		// can be referenced from material Custom-node IncludeFilePaths as
		// "/Plugin/Agility/<file>.ush". UE's auto-mount of /Project/ only covers a
		// project-root Shaders/ folder — plugins must register their own mappings here.
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("Agility"));
		if (Plugin.IsValid())
		{
			const FString ShaderDir = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Shaders"));
			AddShaderSourceDirectoryMapping(TEXT("/Plugin/Agility"), ShaderDir);
		}
	}
};

IMPLEMENT_MODULE(FAgilityModule, Agility)
