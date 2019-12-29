// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelLight.generated.h"

USTRUCT(Atomic, BlueprintType)
struct FVoxelLight
{
	GENERATED_BODY()

	UPROPERTY()
	float Ambient[24];

	bool CompareFace(const FVoxelLight& Other, int32 Direction) const
	{
		for (int32 Index = 0; Index < 4; Index++)
		{
			if (Ambient[Direction * 4 + Index] != Other.Ambient[Direction * 4 + Index])
			{
				return false;
			}
		}

		return true;
	}
};