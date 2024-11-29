// Copyright (c) 2024 https://github.com/FexotheFCO/Simple_Car_Physics_UE under MIT License. 
// 
// Made by Agustin "Troskia" Torchia

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"
#include <Kismet/KismetSystemLibrary.h>
#include "TutorialCar.generated.h"

USTRUCT(BlueprintType)
struct FCarWheel
{
	GENERATED_USTRUCT_BODY()

	FCarWheel()
	{};

	FCarWheel(FVector position, bool isSteering, bool isEngine, float grip)
		: LocalPosition(position), IsSteering(isSteering), IsEngine(isEngine), Grip(grip)
	{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LocalPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsSteering;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsEngine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Grip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsGrounded { false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector GroundNormal { FVector::ZeroVector };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CurrentDirection { FVector::ZeroVector };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CurrentWorldPosition { FVector::ZeroVector };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ApplyForcesLocalPosition { FVector::ZeroVector };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ApplyForcesWorldPosition { FVector::ZeroVector };
};

UENUM(BlueprintType)
enum WheelsPositionsEnum
{
	FL,
	FR,
	RL,
	RR,
};

UCLASS()
class ATutorialCar : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATutorialCar();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UBoxComponent* CarBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USpringArmComponent* SpringArmComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UCameraComponent* CameraComp;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FCarWheel> Springs;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Car Configuration")
	float SuspensionStrength = 360000;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Car Configuration")
	float SuspensionLength = 50;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Car Configuration")
	float SuspensionDamping = 1000;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Car Configuration")
	float ForwardForce = 50000;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Car Configuration")
	float SteeringForce = 100;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Car Configuration")
	float MaxAngle = 45;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Car Configuration")
	float CoMVerticalOffset = -20;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float SteeringInput = 0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float AccelerationInput = 0;

	UFUNCTION(BlueprintCallable)
	void SetAccelerationInput(float value) { AccelerationInput = value; }

	UFUNCTION(BlueprintCallable)
	void SetSteeringInput(float value) { SteeringInput = value; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SuspensionTick();

	void AccelerationTick();

	void SteeringTick();
};
