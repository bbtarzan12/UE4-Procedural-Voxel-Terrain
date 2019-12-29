// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerrainWorkerInformation.h"
#include "../VoxelLight.h"

DECLARE_STATS_GROUP(TEXT("FVoxelTerrainWorker"), STATGROUP_FVoxelTerrainWorker, STATCAT_Advanced);

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
	void BuildLight(FTerrainWorkerInformation& Information);
	bool CalculateAmbient(TArray<FVoxel>& Voxels, TMap<FIntVector, TArray<FVoxel>>& VoxelsWithNeighbors, FIntVector GridLocation, FIntVector ChunkLocation, FIntVector ChunkSize, float ChunkScale);
	
	void GenerateMesh(FTerrainWorkerInformation& Information);
	void AddQuadByDirection(int32 Direction, uint8 Type, const FVoxelLight& Light, float Width, float Height, FIntVector GridLocation, int32 NumFace, FTerrainWorkerInformation& Information);


	TArray<FVoxelLight> Lights;
	TQueue<FTerrainWorkerInformation> Queue;

	FRunnableThread* Thread;
	FThreadSafeCounter StopCounter;

	static const FVector CubeVertices[];
	static const int32 CubeFaces[];
	static const int32 CubeIndices[];
	static const int32 CubeFlipedIndices[];
	static const FVector2D CubeUVs[];
	static const FIntVector VoxelDirectionOffsets[];
	static const int32 DirectionAlignedX[];
	static const int32 DirectionAlignedY[];
	static const int32 DirectionAlignedZ[];
	static const int32 DirectionAlignedSign[];
	static const int32 AONeighborOffsets[];
	
};
