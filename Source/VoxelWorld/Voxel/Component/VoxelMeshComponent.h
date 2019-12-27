// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "../Voxel.h"
#include "VoxelMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class VOXELWORLD_API UVoxelMeshComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()
	
public:
	void GenerateVoxelMesh(const TArray<FVoxel> Voxels, FIntVector ChunkSize, float Scale);

private:
	TArray<FVector> Vertices;
	TArray<int32> Indices;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	static const FVector CubeVertices[];
	static const int CubeFaces[];
	static const int CubeIndices[];
	static const FIntVector VoxelDirectionOffsets[];

	
};
