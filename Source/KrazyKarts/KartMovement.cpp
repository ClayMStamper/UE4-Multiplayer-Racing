// Fill out your copyright notice in the Description page of Project Settings.


#include "KartMovement.h"

#include "DrawDebugHelpers.h"
#include "GoKart.h"
#include "Net/UnrealNetwork.h"

#define MSG(msg) GEngine->AddOnScreenDebugMessage(0, 5, FColor::Green, msg);

UKartMovement::UKartMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), bReplicatedFlag(1), PlayerInputComponent(nullptr), MagTorque(0), Kart(nullptr)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UKartMovement::BeginPlay()
{
	Super::BeginPlay();
	BindAxis("MoveForward", this, &UKartMovement::Client_AccelerateForward);
	BindAxis("MoveRight", this, &UKartMovement::Client_RotateYaw);
	Kart = Cast<AGoKart>(GetOwner());
	check(Kart);

	Velocity = FVector::ZeroVector;
	Acceleration = FVector::ZeroVector;
	
}

// Called every frame
void UKartMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Create a new move

	// Save list of un-acked moves

	// Send the move to the server

	//Simulate move locally
	
	Accelerate(DeltaTime);
	UpdateTransform();
	Rotate(DeltaTime);

	DrawDebugScreenMessages();
	
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

void UKartMovement::Rotate(const float &DeltaTime)
{
	// construct quat
	const float ForwardSpeed = FVector::DotProduct(Kart->GetActorForwardVector(), Velocity);
	const float AngleOfRotation = ForwardSpeed / TurnRadius * MagTorque;
	const FQuat DeltaRot = FQuat(Kart->GetActorUpVector(), AngleOfRotation * DeltaTime);

	// rotate kart actor
	Kart->AddActorWorldRotation(DeltaRot);

	// rotate velocity
	Velocity = DeltaRot.RotateVector(Velocity);
}

void UKartMovement::UpdateTransform()
{
	if (!Kart)
		return;
	
	// move
	FHitResult Hit;
	Kart->AddActorWorldOffset(Velocity, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}

	// correct position to server replicated pos
	if (Kart->HasAuthority())
	{
		Server_MoveState.Transform = Kart->GetActorTransform();
		Server_MoveState.Velocity = Velocity;
		//TODO: update last move
	} 
}

// when server updates move state, propagate to clients
void UKartMovement::OnRep_ServerMoveState()
{
	if (!Kart)
		return;
	Kart->SetActorTransform(Server_MoveState.Transform);
	Velocity = Server_MoveState.Velocity;
}

void UKartMovement::Client_AccelerateForward(float AxisInput)
{
	if(Kart)
	{
		Acceleration = Kart->GetActorForwardVector() * AxisInput * AccelerationRate;
		Server_AccelerateForward(AxisInput);
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("Kart is null!!"));
	}
}

void UKartMovement::Client_RotateYaw(float AxisInput)
{
	MagTorque = AxisInput;
	Server_RotateYaw(AxisInput);
}

void UKartMovement::Server_AccelerateForward_Implementation(float AxisInput)
{
	if(Kart)
		Acceleration = Kart->GetActorForwardVector() * AxisInput * AccelerationRate;
}

bool UKartMovement::Server_AccelerateForward_Validate(float AxisInput)
{
	return FMath::Abs(AxisInput) <= 1;
}


void UKartMovement::Server_RotateYaw_Implementation(float AxisInput)
{
	MagTorque = AxisInput;
}

bool UKartMovement::Server_RotateYaw_Validate(float AxisInput)
{
	return true;
}

void UKartMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UKartMovement, bReplicatedFlag);
	DOREPLIFETIME(UKartMovement, Server_MoveState);
	
}

void UKartMovement::Server_OnRecieveMove_Implementation()
{
	// check valid

	// simulate move

	// send canonical state
}

bool UKartMovement::Server_OnRecieveMove_Validate()
{
	return true;
}

void UKartMovement::Client_OnRecieveMoveState_Implementation()
{
	// remove all moves included in state

	// reset to server state

	// replace/simulate unacked moves
}

bool UKartMovement::Client_OnRecieveMoveState_Validate()
{
	return true;
}

// for use in blueprint speedometer
float UKartMovement::GetSpeed()
{
	return Velocity.Size();
}

FString UKartMovement::RoleEnumToText(ENetRole Role)
{
	switch (Role)
	{
		case ROLE_None:
			return "None";
		case ROLE_SimulatedProxy:
			return "Proxy: Simulated";
		case ROLE_AutonomousProxy:
			return "Proxy: Autonomous";
		case ROLE_Authority:
			return "Authority";
		default:
			return "ENetRole not found";
	}
}

void UKartMovement::DrawDebugScreenMessages() 
{
	DrawDebugString
    (
        GetWorld(),
        Kart->GetActorLocation() + Kart->GetActorUpVector() * 50,
        FString::Printf(TEXT("MPH: %f"), Velocity.Size()),
        0,
        FColor::Red,
        .0001f
    );

	DrawDebugString
    (
        GetWorld(),
        Kart->GetActorLocation() + Kart->GetActorUpVector() * 100,
        FString::Printf(TEXT("Role: ")) + RoleEnumToText(GetOwnerRole()),
        0,
        FColor::Red,
        .0001f
    );
}


