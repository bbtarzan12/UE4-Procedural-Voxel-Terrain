// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelTerrainWorker.h"
#include "../Component/VoxelMeshComponent.h"
#include "../Util/VoxelUtil.h"

FVoxelTerrainWorker* FVoxelTerrainWorker::Runnable = nullptr;

FVoxelTerrainWorker::FVoxelTerrainWorker()
	: StopCounter(0)
{
	Thread = FRunnableThread::Create(this, TEXT("FVoxelTerrainWorker"), 0, TPri_BelowNormal);
}

FVoxelTerrainWorker::~FVoxelTerrainWorker()
{
	delete Thread;
	Thread = nullptr;
}

bool FVoxelTerrainWorker::Init()
{
	return true;
}

uint32 FVoxelTerrainWorker::Run()
{
	while (StopCounter.GetValue() == 0)
	{
		if (Queue.IsEmpty())
		{
			FPlatformProcess::Sleep(0.1f);
			continue;
		}

		FTerrainWorkerInformation Information;
		Queue.Dequeue(Information);

		int NumFaces = 0;
		const FIntVector& ChunkSize = Information.ChunkSize;
		const TArray<FVoxel>& Voxels = Information.Voxels;

		for (int32 VoxelIndex = 0; VoxelIndex < ChunkSize.X * ChunkSize.Y * ChunkSize.Z; VoxelIndex++)
		{
			if (Voxels[VoxelIndex].Type == 0)
				continue;

			FIntVector GridLocation = UVoxelUtil::To3DIndex(VoxelIndex, ChunkSize);

			for (int32 Direction = 0; Direction < 6; Direction++)
			{
				FIntVector NeighborLocation = GridLocation + VoxelDirectionOffsets[Direction];

				if (UVoxelUtil::TransparencyCheck(Voxels, NeighborLocation, ChunkSize))
					continue;

				for (int32 Index = 0; Index < 4; Index++)
				{
					FVector Vertex = (CubeVertices[CubeFaces[Index + Direction * 4]] + FVector(GridLocation)) * Information.ChunkScale;
					Information.Vertices.Add(Vertex);
					Information.Normals.Add(FVector(VoxelDirectionOffsets[Direction]));
				}

				for (int Index = 0; Index < 6; Index++)
				{
					Information.Indices.Add(CubeIndices[Direction * 6 + Index] + NumFaces * 4);
				}
				NumFaces++;
			}
		}

		if (StopCounter.GetValue() == 0 && Information.MeshComponent)
		{
			Information.MeshComponent->FinishWork(Information);
		}
	}
	return 0;
}

void FVoxelTerrainWorker::Stop()
{
	StopCounter.Increment();
	Thread->WaitForCompletion();
}

void FVoxelTerrainWorker::Enqueue(const FTerrainWorkerInformation& Information)
{
	if (!Runnable)
	{
		Runnable = new FVoxelTerrainWorker();
	}
	Runnable->Queue.Enqueue(Information);
}

void FVoxelTerrainWorker::Shutdown()
{
	if (Runnable)
	{
		Runnable->Stop();
		delete Runnable;
		Runnable = nullptr;
	}
}


const FVector FVoxelTerrainWorker::CubeVertices[]
{
	FVector(0.f, 0.f, 0.f),
	FVector(1.f, 0.f, 0.f),
	FVector(1.f, 1.f, 0.f),
	FVector(0.f, 1.f, 0.f),
	FVector(0.f, 0.f, 1.f),
	FVector(1.f, 0.f, 1.f),
	FVector(1.f, 1.f, 1.f),
	FVector(0.f, 1.f, 1.f)
};

const int FVoxelTerrainWorker::CubeFaces[]
{
	1, 2, 5, 6, // front
	3, 0, 7, 4, // back
	2, 3, 6, 7, // right
	0, 1, 4, 5, // left
	4, 5, 7, 6, // top
	3, 2, 0, 1, // bottom
};

const int FVoxelTerrainWorker::CubeIndices[]
{
	0, 3, 1,
	0, 2, 3, //face front
	0, 3, 1,
	0, 2, 3, //face back
	0, 3, 1,
	0, 2, 3, //face right
	0, 3, 1,
	0, 2, 3, //face left
	0, 3, 1,
	0, 2, 3, //face top
	0, 3, 1,
	0, 2, 3, //face bottom
};

const FIntVector FVoxelTerrainWorker::VoxelDirectionOffsets[]
{
	FIntVector{1, 0, 0}, // front
	FIntVector{-1, 0, 0}, // back
	FIntVector{0, 1, 0}, // right
	FIntVector{0, -1, 0}, // left
	FIntVector{0, 0, 1}, // top
	FIntVector{0, 0, -1}, // bottom
};
