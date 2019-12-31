// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <ProceduralMeshComponent.h>
#include "../Voxel.h"
#include "TerrainWorkerInformation.generated.h"

USTRUCT(BlueprintType)
struct FTerrainWorkerInformation
{
	GENERATED_BODY()
	
	TMap<FIntVector, TArray<FVoxel>> VoxelsWithNeighbors;

	UPROPERTY(BlueprintReadWrite)
	class AVoxelChunk* Chunk;

	UPROPERTY(BlueprintReadWrite)
	FIntVector ChunkLocation;

	UPROPERTY(BlueprintReadWrite)
	FIntVector ChunkSize;

	UPROPERTY(BlueprintReadWrite)
	float ChunkScale;

	UPROPERTY(BlueprintReadWrite)
	TArray<FVector> Vertices;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> Indices;

	UPROPERTY(BlueprintReadWrite)
	TArray<FVector> Normals;

	UPROPERTY(BlueprintReadWrite)
	TArray<FVector2D> UV0;

	UPROPERTY(BlueprintReadWrite)
	TArray<FVector2D> UV1;

	UPROPERTY(BlueprintReadWrite)
	TArray<FVector2D> UV2;

	UPROPERTY(BlueprintReadWrite)
	TArray<FVector2D> UV3;

	UPROPERTY(BlueprintReadWrite)
	TArray<FLinearColor> VertexColors;

	UPROPERTY(BlueprintReadWrite)
	TArray<FProcMeshTangent> Tangents;
};