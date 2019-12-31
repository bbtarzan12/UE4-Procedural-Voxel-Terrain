// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelTerrainWorker.h"
#include "../Util/VoxelUtil.h"
#include "../VoxelChunk.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelTerrainWorker ~ BuildLight"), STAT_BuildLight, STATGROUP_FVoxelTerrainWorker);
DECLARE_CYCLE_STAT(TEXT("FVoxelTerrainWorker ~ GenerateMesh"), STAT_GenerateMesh, STATGROUP_FVoxelTerrainWorker);


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

		BuildLight(Information);
		GenerateMesh(Information);

		if (StopCounter.GetValue() == 0 && Information.Chunk)
		{
			Information.Chunk->FinishWork(Information);
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


void FVoxelTerrainWorker::BuildLight(FTerrainWorkerInformation& Information)
{
	SCOPE_CYCLE_COUNTER(STAT_BuildLight);
	const FIntVector& ChunkSize = Information.ChunkSize;

	Lights.SetNumUninitialized(ChunkSize.X * ChunkSize.Y * ChunkSize.Z);

	TArray<FVoxel>& Voxels = Information.VoxelsWithNeighbors[Information.ChunkLocation];

	for (int32 X = 0; X < Information.ChunkSize.X; X++)
	{
		for (int32 Y = 0; Y < Information.ChunkSize.Y; Y++)
		{
			for (int32 Z = 0; Z < Information.ChunkSize.Z; Z++)
			{
				FIntVector GridLocation{ X, Y, Z };
				int32 VoxelIndex = UVoxelUtil::To1DIndex(GridLocation, ChunkSize);

				FVoxel& Voxel = Voxels[VoxelIndex];

				if (Voxel.Type == 0)
				{
					continue;
				}

				for (int32 Direction = 0; Direction < 6; Direction++)
				{
					FIntVector NeighborLocation = GridLocation + VoxelDirectionOffsets[Direction];

					if(UVoxelUtil::OpaqueCheck(Voxels, NeighborLocation, ChunkSize))
						continue;

					// Todo : Code Cleanup!!
					FIntVector Down = GridLocation;
					FIntVector Left = GridLocation;
					FIntVector Top = GridLocation;
					FIntVector Right = GridLocation;
					FIntVector LeftDownCorner = GridLocation;
					FIntVector TopLeftCorner = GridLocation;
					FIntVector TopRightCorner = GridLocation;
					FIntVector RightDownCorner = GridLocation;

					Down[DirectionAlignedY[Direction]] -= 1;
					Left[DirectionAlignedX[Direction]] -= 1;
					Top[DirectionAlignedY[Direction]] += 1;
					Right[DirectionAlignedX[Direction]] += 1;

					LeftDownCorner[DirectionAlignedX[Direction]] -= 1;
					LeftDownCorner[DirectionAlignedY[Direction]] -= 1;

					TopLeftCorner[DirectionAlignedX[Direction]] -= 1;
					TopLeftCorner[DirectionAlignedY[Direction]] += 1;

					TopRightCorner[DirectionAlignedX[Direction]] += 1;
					TopRightCorner[DirectionAlignedY[Direction]] += 1;

					RightDownCorner[DirectionAlignedX[Direction]] += 1;
					RightDownCorner[DirectionAlignedY[Direction]] -= 1;

					FIntVector Neighbors[8]{Down, LeftDownCorner, Left, TopLeftCorner, Top, TopRightCorner, Right, RightDownCorner};

					for (int32 Index = 0; Index < 8; Index++)
					{
						Neighbors[Index][DirectionAlignedZ[Direction]] += DirectionAlignedSign[Direction];
					}

					for (int32 Index = 0; Index < 4; Index++)
					{
						bool Side1 = CalculateAmbient(Voxels, Information.VoxelsWithNeighbors, Neighbors[AONeighborOffsets[Index * 3]], Information.ChunkLocation, ChunkSize, Information.ChunkScale);
						bool Corner = CalculateAmbient(Voxels, Information.VoxelsWithNeighbors, Neighbors[AONeighborOffsets[Index * 3 + 1]], Information.ChunkLocation, ChunkSize, Information.ChunkScale);
						bool Side2 = CalculateAmbient(Voxels, Information.VoxelsWithNeighbors, Neighbors[AONeighborOffsets[Index * 3 + 2]], Information.ChunkLocation, ChunkSize, Information.ChunkScale);

						if (Side1 && Side2)
						{
							Lights[VoxelIndex].Ambient[Index + Direction * 4] = 1.f;
						}
						else
						{
							Lights[VoxelIndex].Ambient[Index + Direction * 4] = ((Side1 ? 1.f : 0.f) + (Side2 ? 1.f : 0.f) + (Corner ? 1.f : 0.f)) / 3.f;
						}
					}
				}
			}
		}
	}
}

bool FVoxelTerrainWorker::CalculateAmbient(TArray<FVoxel>& Voxels, TMap<FIntVector, TArray<FVoxel>>& VoxelsWithNeighbors, FIntVector GridLocation, FIntVector ChunkLocation, FIntVector ChunkSize, float ChunkScale)
{
	if (UVoxelUtil::BoundaryCheck(GridLocation, ChunkSize))
	{
		return Voxels[UVoxelUtil::To1DIndex(GridLocation, ChunkSize)].Type != 0;
	}
	else
	{
		FIntVector WorldGridLocation = UVoxelUtil::GridToWorldGrid(GridLocation, ChunkLocation, ChunkSize);
		FVector WorldLocation = UVoxelUtil::WorldGridToWorld(WorldGridLocation, ChunkScale);
		FIntVector NeighborChunkLocation = UVoxelUtil::WorldToChunk(WorldLocation, ChunkSize, ChunkScale);

		if (VoxelsWithNeighbors.Contains(NeighborChunkLocation))
		{
			TArray<FVoxel>& NeighborVoxels = VoxelsWithNeighbors[NeighborChunkLocation];
			FIntVector NeighborGridLocation = UVoxelUtil::WorldGridToGrid(WorldGridLocation, NeighborChunkLocation, ChunkSize);

			return NeighborVoxels[UVoxelUtil::To1DIndex(NeighborGridLocation, ChunkSize)].Type != 0;
		}
	}
	return false;
}

void FVoxelTerrainWorker::GenerateMesh(FTerrainWorkerInformation& Information)
{
	SCOPE_CYCLE_COUNTER(STAT_GenerateMesh);
	int NumFaces = 0;
	const FIntVector& ChunkSize = Information.ChunkSize;
	const TArray<FVoxel>& Voxels = Information.VoxelsWithNeighbors[Information.ChunkLocation];

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

					int32 VoxelIndex = UVoxelUtil::To1DIndex(GridLocation, ChunkSize);
					
					const FVoxel& Voxel = Voxels[VoxelIndex];

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

					const FVoxelLight& Light = Lights[VoxelIndex];
					FIntVector NeighborLocation = GridLocation + VoxelDirectionOffsets[Direction];

					if (UVoxelUtil::OpaqueCheck(Voxels, NeighborLocation, ChunkSize))
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

						FIntVector NeighborNextLocation = NextLocation + VoxelDirectionOffsets[Direction];
						if (UVoxelUtil::OpaqueCheck(Voxels, NeighborNextLocation, ChunkSize))
							break;

						int32 NextIndex = UVoxelUtil::To1DIndex(NextLocation, ChunkSize);

						const FVoxel& NextVoxel = Voxels[NextIndex];
						const FVoxelLight& NextLight = Lights[NextIndex];

						if (NextVoxel.Type != Voxel.Type)
							break;

						if (!NextLight.CompareAmbient(Light, Direction))
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

							FIntVector NeighborNextLocation = NextLocation + VoxelDirectionOffsets[Direction];
							if (UVoxelUtil::OpaqueCheck(Voxels, NeighborNextLocation, ChunkSize))
							{
								bDone = true;
								break;
							}

							int32 NextIndex = UVoxelUtil::To1DIndex(NextLocation, ChunkSize);

							const FVoxel& NextVoxel = Voxels[NextIndex];
							const FVoxelLight& NextLight = Lights[NextIndex];

							if (NextVoxel.Type != Voxel.Type || VisitedSet.Contains(NextLocation) || !NextLight.CompareAmbient(Light, Direction))
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

					AddQuadByDirection(Direction, Voxel.Type, Light, Width, Height, GridLocation, NumFaces, Information);
					Y += Height;
					NumFaces++;
				}
			}
		}
	}
}

void FVoxelTerrainWorker::AddQuadByDirection(int32 Direction, uint8 Type, const FVoxelLight& Light, float Width, float Height, FIntVector GridLocation, int32 NumFace, FTerrainWorkerInformation& Information)
{
	for (int32 Index = 0; Index < 4; Index++)
	{
		FVector Vertex = CubeVertices[CubeFaces[Index + Direction * 4]];
		Vertex[DirectionAlignedX[Direction]] *= Width;
		Vertex[DirectionAlignedY[Direction]] *= Height;
		Vertex = (Vertex + FVector(GridLocation)) * Information.ChunkScale;

		Information.Vertices.Add(Vertex);
		Information.Normals.Emplace(VoxelDirectionOffsets[Direction]);
		Information.UV0.Emplace(CubeUVs[Index].X * Width, CubeUVs[Index].Y * Height);
		Information.UV1.Emplace(Type,0);
		Information.VertexColors.Emplace(0, 0, 0, Light.Ambient[Index + Direction * 4]);
	}

	for (int Index = 0; Index < 6; Index++)
	{
		if (Light.Ambient[Direction * 4] + Light.Ambient[Direction * 4 + 3] > Light.Ambient[Direction * 4 + 1] + Light.Ambient[Direction * 4 + 2])
		{
			Information.Indices.Add(CubeFlipedIndices[Direction * 6 + Index] + NumFace * 4);
		}
		else
		{
			Information.Indices.Add(CubeIndices[Direction * 6 + Index] + NumFace * 4);
		}
	}
}

const FVector FVoxelTerrainWorker::CubeVertices[]
{
	{0.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{1.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 0.f, 1.f},
	{1.f, 0.f, 1.f},
	{1.f, 1.f, 1.f},
	{0.f, 1.f, 1.f}
};

const int32 FVoxelTerrainWorker::CubeFaces[]
{
	1, 2, 5, 6, // front
	0, 3, 4, 7, // back
	3, 2, 7, 6, // right
	0, 1, 4, 5, // left
	4, 5, 7, 6, // top
	0, 1, 3, 2, // bottom
};

const int32 FVoxelTerrainWorker::CubeIndices[]
{
	0, 3, 1,
	0, 2, 3, //face front
	0, 1, 3,
	0, 3, 2, //face back
	0, 1, 3,
	0, 3, 2, //face right
	0, 3, 1,
	0, 2, 3, //face left
	0, 3, 1,
	0, 2, 3, //face top
	0, 1, 3,
	0, 3, 2, //face bottom
};

const int32 FVoxelTerrainWorker::CubeFlipedIndices[]
{
	1, 0, 2,
	1, 2, 3, //face front
	1, 2, 0,
	1, 3, 2, //face back
	1, 2, 0,
	1, 3, 2, //face right
	1, 0, 2,
	1, 2, 3, //face left
	1, 0, 2,
	1, 2, 3, //face top
	1, 2, 0,
	1, 3, 2, //face bottom
};

const FVector2D FVoxelTerrainWorker::CubeUVs[]
{
	{1.0f, 1.0f},
	{0.0f, 1.0f},
	{1.0f, 0.0f},
	{0.0f, 0.0f},
};

const FIntVector FVoxelTerrainWorker::VoxelDirectionOffsets[]
{
	{1, 0, 0}, // front
	{-1, 0, 0}, // back
	{0, 1, 0}, // right
	{0, -1, 0}, // left
	{0, 0, 1}, // top
	{0, 0, -1}, // bottom
};


const int32 FVoxelTerrainWorker::DirectionAlignedSign[] {1, -1, 1, -1, 1, -1};
const int32 FVoxelTerrainWorker::DirectionAlignedX[] {1, 1, 0, 0, 0, 0};
const int32 FVoxelTerrainWorker::DirectionAlignedY[] {2, 2, 2, 2, 1, 1};
const int32 FVoxelTerrainWorker::DirectionAlignedZ[] {0, 0, 1, 1, 2, 2};

const int32 FVoxelTerrainWorker::AONeighborOffsets[]
{
	0, 1, 2,
	6, 7, 0,
	2, 3, 4,
	4, 5, 6,
};