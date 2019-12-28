// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerrainWorkerInformation.h"

class VOXELWORLD_API FVoxelTerrainWorker : public FRunnable
{
	
public:
	static FVoxelTerrainWorker* Runnable;

	FVoxelTerrainWorker();
	virtual ~FVoxelTerrainWorker();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

	static void Enqueue(const FTerrainWorkerInformation& Information);
	static void Shutdown();

private:


	TQueue<FTerrainWorkerInformation> Queue;

	FRunnableThread* Thread;
	FThreadSafeCounter StopCounter;

	static const FVector CubeVertices[];
	static const int CubeFaces[];
	static const int CubeIndices[];
	static const FIntVector VoxelDirectionOffsets[];
	
};
