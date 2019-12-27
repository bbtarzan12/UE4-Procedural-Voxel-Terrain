// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Voxel.generated.h"

USTRUCT(Atomic, BlueprintType)
struct FVoxel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	uint8 Type;
};