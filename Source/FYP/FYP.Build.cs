using UnrealBuildTool;

public class FYP : ModuleRules
{
	public FYP(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"GameplayTags",
			"MotionWarping" // Added for Execution requirement
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
