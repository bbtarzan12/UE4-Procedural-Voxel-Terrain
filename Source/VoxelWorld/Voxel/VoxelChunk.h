// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Voxel.h"
#include "VoxelChunk.generated.h"

class UVoxelTerrainGenerator;

UCLASS()
class VOXELWORLD_API AVoxelChunk : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVoxelChunk();

	void Init(FIntVector ChunkLocation, UVoxelTerrainGenerator* TerrainGenerator);

	void GenerateVoxels();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Voxel)
	class UVoxelMeshComponent* VoxelMeshComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Voxel)
	class UVoxelTerrainGenerator* Generator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Voxel)
	FIntVector ChunkLocation;

	UPROPERTY()
	class UFastNoise* Noise;

	UPROPERTY()
	TArray<FVoxel> Voxels;

};
