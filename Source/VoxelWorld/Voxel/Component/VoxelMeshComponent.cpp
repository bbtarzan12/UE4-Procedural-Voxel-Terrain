// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelMeshComponent.h"
#include "../Util/VoxelUtil.h"

void UVoxelMeshComponent::GenerateVoxelMesh(const TArray<FVoxel> Voxels, FIntVector ChunkSize, float Scale)
{
	int NumFaces = 0;
	for (int32 Index = 0; Index < ChunkSize.X * ChunkSize.Y * ChunkSize.Z; Index++)
	{
		if(Voxels[Index].Type == 0)
			continue;

		FIntVector GridLocation = UVoxelUtil::To3DIndex(Index, ChunkSize);

		for (int32 Direction = 0; Direction < 6; Direction++)
		{
			FIntVector NeighborLocation = GridLocation + VoxelDirectionOffsets[Direction];

			if(UVoxelUtil::TransparencyCheck(Voxels, NeighborLocation, ChunkSize))
				continue;

			for (int32 I = 0; I < 4; I++)
			{
				FVector Vertex = (CubeVertices[CubeFaces[I + Direction * 4]] + FVector(GridLocation)) * Scale;
				Vertices.Add(Vertex);
				Normals.Add(FVector(VoxelDirectionOffsets[Direction]));
			}

			for (int J = 0; J < 6; J++)
			{
				Indices.Add(CubeIndices[Direction * 6 + J] + NumFaces * 4);
			}
			NumFaces++;
		}
	}

	CreateMeshSection_LinearColor(0, Vertices, Indices, Normals, UVs, VertexColors, Tangents, true);
}

const FVector UVoxelMeshComponent::CubeVertices[]
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

const int UVoxelMeshComponent::CubeFaces[]
{
	1, 2, 5, 6, // front
	3, 0, 7, 4, // back
	2, 3, 6, 7, // right
	0, 1, 4, 5, // left
	4, 5, 7, 6, // top
	3, 2, 0, 1, // bottom
};

const int UVoxelMeshComponent::CubeIndices[]
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

const FIntVector UVoxelMeshComponent::VoxelDirectionOffsets[]
{
	FIntVector{1, 0, 0}, // front
	FIntVector{-1, 0, 0}, // back
	FIntVector{0, 1, 0}, // right
	FIntVector{0, -1, 0}, // left
	FIntVector{0, 0, 1}, // top
	FIntVector{0, 0, -1}, // bottom
};
