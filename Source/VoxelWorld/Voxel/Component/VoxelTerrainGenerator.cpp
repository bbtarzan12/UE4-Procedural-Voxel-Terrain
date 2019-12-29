// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelTerrainGenerator.h"
#include <GameFramework/PlayerState.h>
#include "../Util/VoxelUtil.h"
#include "../VoxelChunk.h"
#include "../../VoxelWorldGameState.h"

// Sets default values for this component's properties
UVoxelTerrainGenerator::UVoxelTerrainGenerator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

// Called every frame
void UVoxelTerrainGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	GenerateTerrain();
	ProcessChunkQueue();
}

void UVoxelTerrainGenerator::GenerateTerrain()
{
	if (AVoxelWorldGameState* GameState = Cast<AVoxelWorldGameState>(GetOwner()))
	{
		const TArray<APlayerState*>& PlayerArray = GameState->PlayerArray;

		TSet<FIntVector> PlayerLocations;
		for (auto& PlayerState : PlayerArray)
		{
			if (APawn* Pawn = PlayerState->GetPawn())
			{
				FIntVector ChunkLocation = UVoxelUtil::WorldToChunk(Pawn->GetActorLocation(), ChunkSize, ChunkScale);
				PlayerLocations.Add(ChunkLocation);
			}
		}

		for (auto& PlayerLocation : PlayerLocations)
		{
			if (LastPlayerLocations.Contains(PlayerLocation))
				continue;

			for (int32 X = PlayerLocation.X - ChunkSpawnSize.X; X <= PlayerLocation.X + ChunkSpawnSize.X; X++)
			{
				for (int32 Y = PlayerLocation.Y - ChunkSpawnSize.Y; Y <= PlayerLocation.Y + ChunkSpawnSize.Y; Y++)
				{
					FIntVector ChunkLocation{ X, Y, 0 };

					if (Chunks.Contains(ChunkLocation))
						continue;

					ChunkQueue.Enqueue(ChunkLocation);
				}
			}
		}

		LastPlayerLocations.Empty(PlayerLocations.Num());
		LastPlayerLocations = MoveTempIfPossible(PlayerLocations);
	}
}

void UVoxelTerrainGenerator::ProcessChunkQueue()
{
	int32 NumGeneratedChunks = 0;
	while (ChunkQueue.IsEmpty() == false)
	{
		if (NumGeneratedChunks >= MaxGenerateChunksInFrame)
			break;

		FIntVector ChunkLocation;
		ChunkQueue.Dequeue(ChunkLocation);

		GenerateChunk(ChunkLocation);

		NumGeneratedChunks++;
	}
}

void UVoxelTerrainGenerator::GenerateChunk(FIntVector ChunkLocation)
{
	if (Chunks.Contains(ChunkLocation))
		return;

	FVector WorldLocation = UVoxelUtil::ChunkToWorld(ChunkLocation, ChunkSize, ChunkScale);
	if(AVoxelChunk* Chunk = GetWorld()->SpawnActor<AVoxelChunk>(WorldLocation, FQuat::Identity.Rotator()))
	{
		Chunk->Init(ChunkLocation, this);
		Chunks.Add(ChunkLocation, Chunk);
	}
}