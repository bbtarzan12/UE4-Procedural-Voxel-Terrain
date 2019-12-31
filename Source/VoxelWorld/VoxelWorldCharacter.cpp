// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "VoxelWorldCharacter.h"
#include "VoxelWorldProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "Voxel/VoxelChunk.h"
#include "VoxelWorldGameState.h"
#include "Voxel/Component/VoxelTerrainGenerator.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AVoxelWorldCharacter

AVoxelWorldCharacter::AVoxelWorldCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
}

void AVoxelWorldCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AVoxelWorldCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("PlaceVoxel", IE_Pressed, this, &AVoxelWorldCharacter::OnPlaceVoxel);
	PlayerInputComponent->BindAction("RemoveVoxel", IE_Pressed, this, &AVoxelWorldCharacter::OnRemoveVoxel);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AVoxelWorldCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVoxelWorldCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AVoxelWorldCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AVoxelWorldCharacter::LookUpAtRate);
}

void AVoxelWorldCharacter::OnPlaceVoxel()
{
	FHitResult Hit;

	const FVector TraceLocation = FirstPersonCameraComponent->GetComponentLocation();
	const FVector TraceForward = FirstPersonCameraComponent->GetForwardVector();

	const FVector TraceStart = TraceLocation;
	const FVector TraceEnd = TraceLocation + TraceForward * 1000.0f;

	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
	{
		if (!Hit.bBlockingHit)
			return;

		if (Hit.GetActor()->IsA<AVoxelChunk>())
		{
			if (AVoxelWorldGameState* GameState = Cast<AVoxelWorldGameState>(GetWorld()->GetGameState()))
			{
				if (UVoxelTerrainGenerator* Generator = GameState->GetVoxelTerrainGenerator())
				{
					Generator->SetVoxel(Hit.ImpactPoint + Hit.Normal * 0.1f, 3);
				}
			}
		}
	}
}

void AVoxelWorldCharacter::OnRemoveVoxel()
{
	FHitResult Hit;

	const FVector TraceLocation = FirstPersonCameraComponent->GetComponentLocation();
	const FVector TraceForward = FirstPersonCameraComponent->GetForwardVector();

	const FVector TraceStart = TraceLocation;
	const FVector TraceEnd = TraceLocation + TraceForward * 1000.0f;

	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
	{
		if (!Hit.bBlockingHit)
			return;

		if (Hit.GetActor()->IsA<AVoxelChunk>())
		{
			if (AVoxelWorldGameState* GameState = Cast<AVoxelWorldGameState>(GetWorld()->GetGameState()))
			{
				if (UVoxelTerrainGenerator* Generator = GameState->GetVoxelTerrainGenerator())
				{
					Generator->SetVoxel(Hit.ImpactPoint - Hit.Normal * 0.1f, 0);
				}
			}
		}
	}
}

void AVoxelWorldCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AVoxelWorldCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AVoxelWorldCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AVoxelWorldCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}