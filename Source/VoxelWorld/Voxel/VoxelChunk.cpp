// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelChunk.h"
#include "Component/VoxelTerrainGenerator.h"
#include "FastNoise/FastNoise.h"
#include "Util/VoxelUtil.h"
#include "Worker/VoxelTerrainWorker.h"

DECLARE_CYCLE_STAT(TEXT("AVoxelChunk ~ GenerateVoxels"), STAT_GenerateVoxels, STATGROUP_AVoxelChunk);

// Sets default values
AVoxelChunk::AVoxelChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickInterval = 0.1f;

	static ConstructorHelpers::FObjectFinder<UMaterialInstance> MaterialObject(TEXT("MaterialInstanceConstant'/Game/VoxelWorld/Material/MI_Voxel'"));

	MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	MeshComponent->bUseAsyncCooking = true;

	if (MaterialObject.Object)
	{
		MeshComponent->SetMaterial(0, MaterialObject.Object);
		MeshComponent->SetMobility(EComponentMobility::Static);
	}

	Noise = NewObject<UFastNoise>();

	RootComponent = MeshComponent;
}

void AVoxelChunk::Init(FIntVector Location, UVoxelTerrainGenerator* TerrainGenerator)
{
	SetActorLabel(Location.ToString() + TEXT(" - Mesh X"));
	ChunkLocation = Location;
	Generator = TerrainGenerator;
	GenerateVoxels();
	bDirty = true;
}

void AVoxelChunk::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Generator == nullptr)
		return;

	if (bDirty)
	{
		if (CheckNeighborChunksAllExists() == false)
			return;

		MeshQueue.Empty();
		GenerateMesh();
	}
	else
	{
		if (MeshQueue.IsEmpty())
			return;

		FTerrainWorkerInformation Information;
		while (MeshQueue.Dequeue(Information)) {}
		MeshComponent->CreateMeshSection_LinearColor(0, Information.Vertices, Information.Indices, Information.Normals, Information.UV0, Information.UV1, Information.UV2, Information.UV3, Information.VertexColors, Information.Tangents, true);
	}
}

bool AVoxelChunk::SetVoxel(FIntVector GridLocation, uint8 Type)
{
	if (UVoxelUtil::BoundaryCheck(GridLocation, Generator->ChunkSize))
	{
		Voxels[UVoxelUtil::To1DIndex(GridLocation, Generator->ChunkSize)].Type = Type;
		bDirty = true;
		return true;
	}

	return false;
}

void AVoxelChunk::FinishWork(const FTerrainWorkerInformation& Information)
{
	MeshQueue.Enqueue(Information);
}

void AVoxelChunk::GenerateMesh()
{
	SetActorLabel(ChunkLocation.ToString());

	TMap<FIntVector, TArray<FVoxel>> VoxelsWithNeighbors;

	for (int32 X = ChunkLocation.X - 1; X <= ChunkLocation.X + 1; X++)
	{
		for (int32 Y = ChunkLocation.Y - 1; Y <= ChunkLocation.Y + 1; Y++)
		{
			FIntVector NeighborChunkLocation{ X, Y, 0 };
			VoxelsWithNeighbors.Add(NeighborChunkLocation, Generator->Chunks[NeighborChunkLocation]->Voxels);
		}
	}

	FTerrainWorkerInformation Information;

	Information.VoxelsWithNeighbors = VoxelsWithNeighbors;
	Information.Chunk = this;
	Information.ChunkSize = Generator->ChunkSize;
	Information.ChunkScale = Generator->ChunkScale;
	Information.ChunkLocation = ChunkLocation;

	FVoxelTerrainWorker::Enqueue(Information);
	bDirty = false;
}

bool AVoxelChunk::CheckNeighborChunksAllExists()
{
	if (Generator == nullptr)
		return false;

	for (int32 X = ChunkLocation.X - 1; X <= ChunkLocation.X + 1; X++)
	{
		for (int32 Y = ChunkLocation.Y - 1; Y <= ChunkLocation.Y + 1; Y++)
		{
			if (Generator->Chunks.Contains({ X, Y, 0 }))
			{
				continue;
			}

			return false;
		}
	}

	return true;
}

void AVoxelChunk::GenerateVoxels()
{
	SCOPE_CYCLE_COUNTER(STAT_GenerateVoxels);

	if (Generator == nullptr)
		return;

	const FIntVector& ChunkSize = Generator->ChunkSize;

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

				if (density >= 2)
				{
					Voxels[Index] = FVoxel{ 3 };
				}
				else if (density >= 1)
				{
					Voxels[Index] = FVoxel{ 2 };
				}
				else if (density >= 0)
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
}
