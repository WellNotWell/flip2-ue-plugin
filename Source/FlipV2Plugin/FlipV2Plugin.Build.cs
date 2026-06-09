using UnrealBuildTool;
using System.IO;

public class FlipV2Plugin : ModuleRules
{
	public FlipV2Plugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"RenderCore",
			"RHI"
		});

		string FrameworkRoot = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "flip-framework");
		PrivateIncludePaths.Add(Path.Combine(FrameworkRoot, "flip_tool_2", "src"));
		PrivateIncludePaths.Add(Path.Combine(FrameworkRoot, "flip_image"));

		// Suppress warnings from third-party FLIP headers
		bEnableUndefinedIdentifierWarnings = false;
		bEnableExceptions = true;
	}
}
