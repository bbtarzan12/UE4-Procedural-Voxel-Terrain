// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "VoxelWorldHUD.generated.h"

UCLASS()
class AVoxelWorldHUD : public AHUD
{
	GENERATED_BODY()

public:
	AVoxelWorldHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

