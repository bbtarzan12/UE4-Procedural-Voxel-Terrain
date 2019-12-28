// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelMeshComponent.h"
#include "../Util/VoxelUtil.h"
#include "../Worker/VoxelTerrainWorker.h"

UVoxelMeshComponent::UVoxelMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.1f;
	bUseAsyncCooking = true;
}

void UVoxelMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MeshQueue.IsEmpty())
		return;

	FTerrainWorkerInformation Information;
	MeshQueue.Dequeue(Information);
	CreateMeshSection_LinearColor(0, Information.Vertices, Information.Indices, Information.Normals, Information.UVs, Information.VertexColors, Information.Tangents, true);
}

void UVoxelMeshComponent::GenerateVoxelMesh(const TArray<FVoxel> Voxels, FIntVector ChunkSize, float ChunkScale)
{
	FTerrainWorkerInformation Information;

	Information.Voxels = Voxels;
	Information.MeshComponent = this;
	Information.ChunkSize = ChunkSize;
	Information.ChunkScale = ChunkScale;

	FVoxelTerrainWorker::Enqueue(Information);
}

void UVoxelMeshComponent::FinishWork(const FTerrainWorkerInformation& Information)
{
	MeshQueue.Enqueue(Information);
}