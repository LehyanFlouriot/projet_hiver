// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjetHiver : ModuleRules
{
	public ProjetHiver(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "EnhancedInput", "InputCore", "Json", "JsonUtilities", "HeadMountedDisplay", "OnlineSubsystem", "OnlineSubsystemNull", "OnlineSubsystemSteam", "HTTP", "Sockets", "Niagara", "LevelSequence", "MovieScene", "AkAudio" });
        PrivateDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "GameplayTags", "GameplayTasks" });
    }
}
