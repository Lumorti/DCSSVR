// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class DCSSVRTarget : TargetRules
{
	public DCSSVRTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "DCSSVR" } );

		// BuildEnvironment = TargetBuildEnvironment.Unique;
	    // bUseLoggingInShipping = true;

	}
}
