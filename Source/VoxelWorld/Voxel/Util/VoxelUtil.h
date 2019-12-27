// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Voxel.h"
#include "VoxelUtil.generated.h"

/**
 * 
 */
UCLASS()
class VOXELWORLD_API UVoxelUtil : public UObject
{
	GENERATED_BODY()
	
public:

	static FORCEINLINE FIntVector To3DIndex(int Index, FIntVector ChunkSize)
	{
		return FIntVector
		{
			Index / (ChunkSize.Y * ChunkSize.Z),
			(Index / ChunkSize.Z) % ChunkSize.Y ,
			Index % ChunkSize.Z
		};
	}

	static FORCEINLINE int32 To1DIndex(FIntVector Index, FIntVector ChunkSize)
	{
		return Index.Z + Index.Y * ChunkSize.Z + Index.X * ChunkSize.Y * ChunkSize.Z;
	}

	static FORCEINLINE bool BoundaryCheck(FIntVector GridLocation, FIntVector ChunkSize)
	{
		return ChunkSize.X > GridLocation.X && ChunkSize.Y > GridLocation.Y && ChunkSize.Z > GridLocation.Z && GridLocation.X >= 0 && GridLocation.Y >= 0 && GridLocation.Z >= 0;
	}

	static FORCEINLINE bool TransparencyCheck(const TArray<FVoxel>& Voxels, FIntVector GridLocation, FIntVector ChunkSize)
	{
		if (!BoundaryCheck(GridLocation, ChunkSize))
			return false;

		return Voxels[To1DIndex(GridLocation, ChunkSize)].Type != 0;
	}

	static FORCEINLINE FIntVector WorldToChunk(FVector WorldLocation, FIntVector ChunkSize, float ChunkScale)
	{
		return FIntVector
		{
			FMath::FloorToInt(WorldLocation.X / ChunkSize.X / ChunkScale),
			FMath::FloorToInt(WorldLocation.Y / ChunkSize.Y / ChunkScale),
			FMath::FloorToInt(WorldLocation.Z / ChunkSize.Z / ChunkScale)
		};
	}

	static FORCEINLINE FVector ChunkToWorld(FIntVector ChunkLocation, FIntVector ChunkSize, float ChunkScale)
	{
		return FVector
		{
			ChunkLocation.X * ChunkSize.X * ChunkScale,
			ChunkLocation.Y * ChunkSize.Y * ChunkScale,
			ChunkLocation.Z * ChunkSize.Z * ChunkScale
		};
	}

	static FORCEINLINE FIntVector GridToWorldGrid(FIntVector GridLocation, FIntVector ChunkLocation, FIntVector ChunkSize)
	{
		return FIntVector
		{
			GridLocation.X + ChunkLocation.X * ChunkSize.X,
			GridLocation.Y + ChunkLocation.Y * ChunkSize.Y,
			GridLocation.Z + ChunkLocation.Z * ChunkSize.Z
		};
	}
	
};
