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

		string ThirdPartyPath = Path.Combine(ModuleDirectory, "ThirdParty");
		PrivateIncludePaths.Add(ThirdPartyPath);

		// Suppress warnings from third-party FLIP headers
		bEnableUndefinedIdentifierWarnings = false;
		bEnableExceptions = true;
	}
}
