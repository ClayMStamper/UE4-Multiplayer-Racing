// Fill out your copyright notice in the Description page of Project Settings.


#include "KartMovement.h"
#include "GoKart.h"

#define MSG(msg) GEngine->AddOnScreenDebugMessage(0, 5, FColor::Green, msg);

// Sets default values for this component's properties
UKartMovement::UKartMovement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	BindAxis("MoveForward", this, &UKartMovement::AccelerateForward);
	BindAxis("MoveRight", this, &UKartMovement::RotateVertical);
	
}


// Called when the game starts
void UKartMovement::BeginPlay()
{
	Super::BeginPlay();

	Kart = Cast<AGoKart>(GetOwner());
	check(Kart);

	Velocity = FVector::ZeroVector;
	Acceleration = FVector::ZeroVector;
	
}

// Called every frame
void UKartMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Accelerate(DeltaTime);
	Move();
	Rotate(DeltaTime);
	
}


void UKartMovement::Accelerate(const float &DeltaTime)
{
	const FVector VelocityNormal = Velocity.GetSafeNormal();
	
	// Apply wind resistance
	const FVector AirResistance = -VelocityNormal * Velocity.SizeSquared() * DragCoefficient; 
	Acceleration += AirResistance * DeltaTime;

	// Apply rolling/friction resistance
	const FVector RollingResistance = -VelocityNormal * -GetWorld()->GetGravityZ() * Kart->GetMass() * RollingFrictionCoefficient;
	Acceleration += RollingResistance * DeltaTime;

	// accelerate from input
	Velocity += Acceleration * DeltaTime;
	
}

void UKartMovement::Move()
{
	// move
	FHitResult Hit;
	Kart->AddActorWorldOffset(Velocity, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

void UKartMovement::Rotate(const float &DeltaTime)
{
	// construct quat
	const float AngleOfRotation = Velocity.Size() / TurnRadius * MagTorque;
	const FQuat DeltaRot = FQuat(Kart->GetActorUpVector(), AngleOfRotation * DeltaTime);

	// rotate kart actor
	Kart->AddActorWorldRotation(DeltaRot);

	// rotate velocity
	Velocity = DeltaRot.RotateVector(Velocity);
}


void UKartMovement::AccelerateForward(float AxisInput)
{
	Acceleration = Kart->GetActorForwardVector() * AxisInput * AccelerationRate;
}

void UKartMovement::RotateVertical(float AxisInput)
{
	MagTorque = AxisInput;
}

// for use in blueprint speedometer
float UKartMovement::GetSpeed()
{
	return Velocity.Size();
}


