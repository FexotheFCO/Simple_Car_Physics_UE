// Copyright (c) 2024 https://github.com/FexotheFCO/Simple_Car_Physics_UE under MIT License. 
// 
// Made by Agustin "Troskia" Torchia

#include "TutorialCar.h"

// Sets default values
ATutorialCar::ATutorialCar()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CarBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Car"));
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));

	SetRootComponent(CarBox);

	SpringArmComp->SetupAttachment(CarBox);
	CameraComp->AttachToComponent(SpringArmComp, FAttachmentTransformRules::KeepRelativeTransform);

	//Setting default properties of the SpringArmComp
	SpringArmComp->bUsePawnControlRotation = false;
	SpringArmComp->bEnableCameraLag = false;
	SpringArmComp->TargetArmLength = 500.0f;
	SpringArmComp->bEnableCameraRotationLag = true;

	CarBox->SetSimulatePhysics(true);
	CarBox->SetCollisionProfileName(FName("Vehicle"));
	CarBox->SetBoxExtent(FVector(120, 60, 40));

	CarBox->SetLinearDamping(0.1);
	CarBox->SetAngularDamping(2.5);
}

// Called when the game starts or when spawned
void ATutorialCar::BeginPlay()
{
	Super::BeginPlay();
	
	FVector boxExtent = CarBox->GetUnscaledBoxExtent();

	Springs.EmplaceAt(WheelsPositionsEnum::FL, FCarWheel(FVector(boxExtent.X, -boxExtent.Y, -boxExtent.Z),false, true, true, 1));
	Springs.EmplaceAt(WheelsPositionsEnum::FR, FCarWheel(FVector(boxExtent.X, boxExtent.Y, -boxExtent.Z), false, true, true, 1));
	Springs.EmplaceAt(WheelsPositionsEnum::RL, FCarWheel(FVector(-boxExtent.X, -boxExtent.Y, -boxExtent.Z), false, false, true, 0.8));
	Springs.EmplaceAt(WheelsPositionsEnum::RR, FCarWheel(FVector(-boxExtent.X, boxExtent.Y, -boxExtent.Z), false, false, true, 0.8));

	FVector localCenterOfMass = CarBox->GetComponentTransform().InverseTransformPosition(CarBox->GetCenterOfMass());

	for (FCarWheel& wheel : Springs)
	{
		wheel.ApplyForcesLocalPosition = FVector(wheel.LocalPosition.X, wheel.LocalPosition.Y, localCenterOfMass.Z + CoMVerticalOffset);
	}
}

// Called every frame
void ATutorialCar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SuspensionTick();
	SteeringTick();
	AccelerationTick();
}

void ATutorialCar::SuspensionTick()
{
	for (FCarWheel& wheel : Springs)
	{
		wheel.CurrentWorldPosition = CarBox->GetComponentTransform().TransformPosition(wheel.LocalPosition);
		wheel.ApplyForcesWorldPosition = CarBox->GetComponentTransform().TransformPosition(wheel.ApplyForcesLocalPosition);
		FVector endPoint = CarBox->GetComponentTransform().TransformVector(FVector::DownVector * SuspensionLength) + wheel.CurrentWorldPosition;
		FHitResult hitResult;
		TArray<AActor*> ignoredActors;
		ignoredActors.Add(this);

		wheel.IsGrounded = UKismetSystemLibrary::LineTraceSingleByProfile(GetWorld(), wheel.CurrentWorldPosition, endPoint, "Vehicle", false, ignoredActors, EDrawDebugTrace::ForOneFrame, hitResult,true);
		if (wheel.IsGrounded)
		{
			float compressRatio = (SuspensionLength - hitResult.Distance) / SuspensionLength;
			float totalForce = (compressRatio * SuspensionStrength) - (SuspensionDamping * CarBox->GetPhysicsLinearVelocityAtPoint(wheel.CurrentWorldPosition).Z);

			wheel.GroundNormal = hitResult.Normal;
			DrawDebugLine(GetWorld(), wheel.CurrentWorldPosition, wheel.CurrentWorldPosition + wheel.GroundNormal * totalForce / SuspensionStrength, FColor::Blue, false, -1, 0U, 10);
			CarBox->AddForceAtLocation(wheel.GroundNormal * totalForce, wheel.CurrentWorldPosition);
		}
	}
}

void ATutorialCar::SteeringTick()
{
	for (FCarWheel& wheel : Springs)
	{
		if (!wheel.IsGrounded)
			continue;

		float angle;
		angle = wheel.IsSteering ? SteeringInput * MaxAngle : 0;
		float angleInRadians = FMath::DegreesToRadians(angle);

		FVector carVelocity = CarBox->GetPhysicsLinearVelocityAtPoint(wheel.CurrentWorldPosition);

		//Get the lateral direction of the Wheel
		wheel.CurrentDirection = FVector(FMath::Cos(angleInRadians), FMath::Sin(angleInRadians), 0);
		FVector perpendicularDirection = FVector(-wheel.CurrentDirection.Y, wheel.CurrentDirection.X, 0);
		perpendicularDirection = CarBox->GetComponentTransform().TransformVector(perpendicularDirection);

		//Calculate the final lateral resistance force
		float sideRatio = FVector::DotProduct(carVelocity, perpendicularDirection);
		FVector lateralResistance = perpendicularDirection * -sideRatio * SteeringForce * wheel.Grip;

		DrawDebugLine(GetWorld(), wheel.ApplyForcesWorldPosition, wheel.ApplyForcesWorldPosition + lateralResistance, FColor::Yellow, false, -1, 0U, 10);
		CarBox->AddForceAtLocation(lateralResistance, wheel.ApplyForcesWorldPosition);
	}
}

void ATutorialCar::AccelerationTick()
{
	for (FCarWheel& wheel : Springs)
	{
		if (!wheel.IsGrounded || !wheel.IsEngine)
			continue;

		FVector accelerationForce = CarBox->GetComponentTransform().TransformVector(wheel.CurrentDirection * ForwardForce * AccelerationInput);
		FVector projectedVector = FVector::VectorPlaneProject(accelerationForce, wheel.GroundNormal);

		DrawDebugLine(GetWorld(), wheel.ApplyForcesWorldPosition, wheel.ApplyForcesWorldPosition + projectedVector, FColor::Red, false, -1, 0U, 10);
		CarBox->AddForceAtLocation(projectedVector, wheel.ApplyForcesWorldPosition);
	}
}