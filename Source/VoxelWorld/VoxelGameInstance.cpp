// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelGameInstance.h"
#include "Voxel/Worker/VoxelTerrainWorker.h"

void UVoxelGameInstance::Shutdown()
{
	Super::Shutdown();

	FVoxelTerrainWorker::Shutdown();
}
