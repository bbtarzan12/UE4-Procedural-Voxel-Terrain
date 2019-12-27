// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelChunk.h"
#include "Component/VoxelMeshComponent.h"
#include "Component/VoxelTerrainGenerator.h"
#include "FastNoise/FastNoise.h"
#include "Util/VoxelUtil.h"

// Sets default values
AVoxelChunk::AVoxelChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	VoxelMeshComponent = CreateDefaultSubobject<UVoxelMeshComponent>(TEXT("VoxelMesh"));

	Noise = NewObject<UFastNoise>();

	RootComponent = VoxelMeshComponent;
}

void AVoxelChunk::Init(FIntVector Location, UVoxelTerrainGenerator* TerrainGenerator)
{
	SetActorLabel(Location.ToString());
	ChunkLocation = Location;
	Generator = TerrainGenerator;
	GenerateVoxels();
}

void AVoxelChunk::GenerateVoxels()
{
	if (Generator == nullptr)
		return;

	const FIntVector& ChunkSize = Generator->ChunkSize;
	const float& ChunkScale = Generator->ChunkScale;

	Voxels.SetNumUninitialized(ChunkSize.X * ChunkSize.Y * ChunkSize.Z);
	for (int32 X = 0; X < ChunkSize.X; X++)
	{
		for (int32 Y = 0; Y < ChunkSize.Y; Y++)
		{
			for (int32 Z = 0; Z < ChunkSize.Z; Z++)
			{
				FIntVector GridLocation{ X, Y, Z };
				FIntVector WorldGridLocation = UVoxelUtil::GridToWorldGrid(GridLocation, ChunkLocation, ChunkSize);
				int Index = UVoxelUtil::To1DIndex(GridLocation, ChunkSize);

				int32 density = -WorldGridLocation.Z + 64;

				density += Noise->GetSimplexFractal(WorldGridLocation.X, WorldGridLocation.Y) * 10.0f;

				if (density >= 0)
				{
					Voxels[Index] = FVoxel{ 1 };
				}
				else
				{
					Voxels[Index] = FVoxel{ 0 };
				}
			}
		}
	}

	VoxelMeshComponent->GenerateVoxelMesh(Voxels, ChunkSize, ChunkScale);
}

// Called when the game starts or when spawned
void AVoxelChunk::BeginPlay()
{
	Super::BeginPlay();
	
}