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
	
	UPROPERTY(BlueprintReadWrite)
	TArray<FVoxel> Voxels;

	UPROPERTY(BlueprintReadWrite)
	class UVoxelMeshComponent* MeshComponent;

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
	TArray<FVector2D> UVs;

	UPROPERTY(BlueprintReadWrite)
	TArray<FLinearColor> VertexColors;

	UPROPERTY(BlueprintReadWrite)
	TArray<FProcMeshTangent> Tangents;
};