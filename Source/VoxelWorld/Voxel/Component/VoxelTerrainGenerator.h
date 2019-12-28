// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelTerrainGenerator.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VOXELWORLD_API UVoxelTerrainGenerator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVoxelTerrainGenerator();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void GenerateTerrain();
	void GenerateChunk(FIntVector ChunkLocation);
	void ProcessChunkQueue();

public:

	TQueue<FIntVector> ChunkQueue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Voxel)
	TSet<FIntVector> LastPlayerLocations;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Voxel)
	TMap<FIntVector, class AVoxelChunk*> Chunks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Voxel)
	FIntVector ChunkSize{ 16, 16, 128};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Voxel)
	FIntPoint ChunkSpawnSize{2, 2};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Voxel)
	float ChunkScale = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Voxel)
	int32 MaxGenerateChunksInFrame = 1;
		
};
