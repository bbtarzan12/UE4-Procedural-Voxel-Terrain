// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "VoxelGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class VOXELWORLD_API UVoxelGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Shutdown() override;
	
};
