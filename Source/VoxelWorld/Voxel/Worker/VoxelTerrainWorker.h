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

	static void Enqueue(FTerrainWorkerInformation Information);
	static void Shutdown();

private:
	void GenerateMesh(FTerrainWorkerInformation& Information);
	void AddQuadByDirection(int32 Direction, uint8 Type, float Width, float Height, FIntVector GridLocation, int32 NumFace, FTerrainWorkerInformation& Information);

	TQueue<FTerrainWorkerInformation> Queue;

	FRunnableThread* Thread;
	FThreadSafeCounter StopCounter;

	static const FVector CubeVertices[];
	static const int32 CubeFaces[];
	static const int32 CubeIndices[];
	static const FVector2D CubeUVs[];
	static const FIntVector VoxelDirectionOffsets[];
	static const int32 DirectionAlignedX[];
	static const int32 DirectionAlignedY[];
	static const int32 DirectionAlignedZ[];
	
};
