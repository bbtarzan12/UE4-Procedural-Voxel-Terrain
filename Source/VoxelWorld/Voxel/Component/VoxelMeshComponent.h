// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "../Voxel.h"
#include "../Worker/TerrainWorkerInformation.h"
#include "VoxelMeshComponent.generated.h"


/**
 * 
 */
UCLASS()
class VOXELWORLD_API UVoxelMeshComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()
	
public:
	UVoxelMeshComponent(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void GenerateVoxelMesh(const TArray<FVoxel> Voxels, FIntVector ChunkSize, float ChunkScale);
	void FinishWork(FTerrainWorkerInformation Information);

private:
	TQueue<FTerrainWorkerInformation> MeshQueue;
	
};
