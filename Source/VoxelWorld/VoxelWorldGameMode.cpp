// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "VoxelWorldGameMode.h"
#include "VoxelWorldHUD.h"
#include "VoxelWorldCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Voxel/VoxelWorldGameState.h"

AVoxelWorldGameMode::AVoxelWorldGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AVoxelWorldHUD::StaticClass();

	GameStateClass = AVoxelWorldGameState::StaticClass();
}
