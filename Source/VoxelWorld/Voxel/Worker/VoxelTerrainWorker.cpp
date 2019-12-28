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

		GenerateMesh(Information);

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


void FVoxelTerrainWorker::GenerateMesh(FTerrainWorkerInformation& Information)
{
	int NumFaces = 0;
	const FIntVector& ChunkSize = Information.ChunkSize;
	const TArray<FVoxel>& Voxels = Information.Voxels;

	for (int32 Direction = 0; Direction < 6; Direction++)
	{		
		for (int32 Depth = 0; Depth < ChunkSize[DirectionAlignedZ[Direction]]; Depth++)
		{
			TSet<FIntVector> VisitedSet;
			for (int32 X = 0; X < ChunkSize[DirectionAlignedX[Direction]]; X++)
			{
				for (int32 Y = 0; Y < ChunkSize[DirectionAlignedY[Direction]];)
				{
					FIntVector GridLocation;
					GridLocation[DirectionAlignedX[Direction]] = X;
					GridLocation[DirectionAlignedY[Direction]] = Y;
					GridLocation[DirectionAlignedZ[Direction]] = Depth;
					
					FVoxel Voxel = Voxels[UVoxelUtil::To1DIndex(GridLocation, ChunkSize)];

					if (Voxel.Type == 0)
					{
						Y++;
						continue;
					}

					if (VisitedSet.Contains(GridLocation))
					{
						Y++;
						continue;
					}

					FIntVector NeighborLocation = GridLocation + VoxelDirectionOffsets[Direction];

					if (UVoxelUtil::TransparencyCheck(Voxels, NeighborLocation, ChunkSize))
					{
						Y++;
						continue;
					}

					VisitedSet.Add(GridLocation);

					int32 Height;
					for (Height = 1; Height + Y < ChunkSize[DirectionAlignedY[Direction]]; Height++)
					{
						FIntVector NextLocation = GridLocation;
						NextLocation[DirectionAlignedY[Direction]] += Height;

						FVoxel NextVoxel = Voxels[UVoxelUtil::To1DIndex(NextLocation, ChunkSize)];

						if (NextVoxel.Type != Voxel.Type)
							break;

						if (VisitedSet.Contains(NextLocation))
							break;

						VisitedSet.Add(NextLocation);
					}

					bool bDone = false;
					int32 Width;
					for (Width = 1; Width + X < ChunkSize[DirectionAlignedX[Direction]]; Width++)
					{
						for (int32 dy = 0; dy < Height; dy++)
						{
							FIntVector NextLocation = GridLocation;
							NextLocation[DirectionAlignedX[Direction]] += Width;
							NextLocation[DirectionAlignedY[Direction]] += dy;

							FVoxel NextVoxel = Voxels[UVoxelUtil::To1DIndex(NextLocation, ChunkSize)];

							if (NextVoxel.Type != Voxel.Type || VisitedSet.Contains(NextLocation))
							{
								bDone = true;
								break;
							}
						}

						if (bDone)
						{
							break;
						}

						for (int32 dy = 0; dy < Height; dy++)
						{
							FIntVector NextLocation = GridLocation;
							NextLocation[DirectionAlignedX[Direction]] += Width;
							NextLocation[DirectionAlignedY[Direction]] += dy;
							VisitedSet.Add(NextLocation);
						}
					}

					AddQuadByDirection(Direction, Voxel.Type, Width, Height, GridLocation, NumFaces, Information);
					Y += Height;
					NumFaces++;
				}
			}
		}
	}
}

void FVoxelTerrainWorker::AddQuadByDirection(int32 Direction, uint8 type, float Width, float Height, FIntVector GridLocation, int32 NumFace, FTerrainWorkerInformation& Information)
{
	for (int32 Index = 0; Index < 4; Index++)
	{
		FVector Vertex = CubeVertices[CubeFaces[Index + Direction * 4]];
		Vertex[DirectionAlignedX[Direction]] *= Width;
		Vertex[DirectionAlignedY[Direction]] *= Height;
		Vertex = (Vertex + FVector(GridLocation)) * Information.ChunkScale;

		Information.Vertices.Add(Vertex);
		Information.Normals.Add(FVector(VoxelDirectionOffsets[Direction]));
	}

	for (int Index = 0; Index < 6; Index++)
	{
		Information.Indices.Add(CubeIndices[Direction * 6 + Index] + NumFace * 4);
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

const int32 FVoxelTerrainWorker::CubeFaces[]
{
	1, 2, 5, 6, // front
	3, 0, 7, 4, // back
	2, 3, 6, 7, // right
	0, 1, 4, 5, // left
	4, 5, 7, 6, // top
	3, 2, 0, 1, // bottom
};

const int32 FVoxelTerrainWorker::CubeIndices[]
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

const int32 FVoxelTerrainWorker::DirectionAlignedX[] {1, 1, 0, 0, 0, 0};
const int32 FVoxelTerrainWorker::DirectionAlignedY[] {2, 2, 2, 2, 1, 1};
const int32 FVoxelTerrainWorker::DirectionAlignedZ[] {0, 0, 1, 1, 2, 2};