// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "VoxelWorldGameState.generated.h"

/**
 * 
 */
UCLASS()
class VOXELWORLD_API AVoxelWorldGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AVoxelWorldGameState();

	FORCEINLINE class UVoxelTerrainGenerator* GetVoxelTerrainGenerator() const { return TerrainGenerator; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Voxel)
	class UVoxelTerrainGenerator* TerrainGenerator;
	
};
