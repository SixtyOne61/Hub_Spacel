// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class Hub_SpacelServerTarget : TargetRules
{
    public Hub_SpacelServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		//DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("Hub_Spacel");
	}
}
