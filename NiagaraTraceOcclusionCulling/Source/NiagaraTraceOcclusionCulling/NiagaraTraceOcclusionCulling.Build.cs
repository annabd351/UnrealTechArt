// For portfolio purposes only -- Not for distribution, all rights reserved

using UnrealBuildTool;

public class NiagaraTraceOcclusionCulling : ModuleRules
{
	public NiagaraTraceOcclusionCulling(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Niagara"
		});
	}
}
