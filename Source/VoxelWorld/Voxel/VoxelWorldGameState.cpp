// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelWorldGameState.h"
#include "Component/VoxelTerrainGenerator.h"

AVoxelWorldGameState::AVoxelWorldGameState()
{
	TerrainGenerator = CreateDefaultSubobject<UVoxelTerrainGenerator>(TEXT("Voxel Terrain Generator"));
}
