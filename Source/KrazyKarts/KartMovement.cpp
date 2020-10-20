// Fill out your copyright notice in the Description page of Project Settings.


#include "KartMovement.h"

#include "DrawDebugHelpers.h"
#include "GoKart.h"
#include "Net/UnrealNetwork.h"

#define MSG(msg) GEngine->AddOnScreenDebugMessage(0, 5, FColor::Green, msg);


UKartMovement::UKartMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
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

}

// input method for acceleration
void UKartMovement::Client_AccelerateForward(float AxisInput)
{
	Throttle = AxisInput;
}

// input method for rotation
void UKartMovement::Client_RotateYaw(float AxisInput)
{
	Torque = AxisInput;
}

// Called every frame
void UKartMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Kart) return;

	if (Kart->IsLocallyControlled())
	{
		const FKartMoveInput MoveInput = CreateMoveInputObject(DeltaTime);
		if (!Kart->HasAuthority())
		{
			UnAckedMoves.Add(MoveInput);
			// UE_LOG(LogTemp, Warning, TEXT("Unacked move count: %d"), UnAckedMoves.Num());
		}
		SimulateMoveLocally(MoveInput);
		Server_ReceiveMoveInput(MoveInput);
	}
	
	DrawDebugScreenMessages();
	
}

FKartMoveInput UKartMovement::CreateMoveInputObject(const float& DeltaTime) const
{
	FKartMoveInput MoveInput = FKartMoveInput();
	MoveInput.DeltaTime = DeltaTime;
	MoveInput.Throttle = Throttle;
	MoveInput.Torque = Torque;
	MoveInput.TimeStamp = GetWorld()->TimeSeconds;

	return MoveInput;
}

void UKartMovement::ClearAckedMoves(FKartMoveInput LastMove)
{
	TArray<FKartMoveInput> NewMoves;

	for (FKartMoveInput Move : UnAckedMoves)
	{
		if (Move.TimeStamp > LastMove.TimeStamp)
		{
			NewMoves.Add(Move);
		}
	}

	UnAckedMoves = NewMoves;
}

void UKartMovement::SimulateMoveLocally(const FKartMoveInput& MoveInput)
{
	//Simulate move locally
	Accelerate(MoveInput.DeltaTime, MoveInput.Throttle);
	Rotate(MoveInput.DeltaTime, MoveInput.Torque);
	UpdateTransform();
	ReplicatedState.LastMove = MoveInput;	
}

void UKartMovement::Server_ReceiveMoveInput_Implementation(const FKartMoveInput& MoveInput)
{
	// Match this authoritative entity's move to autonomous proxy's input
	Throttle = MoveInput.Throttle;
	Torque = MoveInput.Torque;
	
	SimulateMoveLocally(MoveInput);

}

bool UKartMovement::Server_ReceiveMoveInput_Validate(const FKartMoveInput& MoveInput)
{
	return true;
}

void UKartMovement::Accelerate(const float &DeltaTime, const float& ThrottleInput)
{
	const FVector VelocityNormal = Velocity.GetSafeNormal();

	// set acceleration from input
	Acceleration = Kart->GetActorForwardVector() * ThrottleInput * AccelerationScalar;

	// Apply wind resistance
	const FVector AirResistance = -VelocityNormal * Velocity.SizeSquared() * DragCoefficient; 
	Acceleration += AirResistance * DeltaTime;

	// Apply rolling/friction resistance
	const FVector RollingResistance = -VelocityNormal * -GetWorld()->GetGravityZ() * Kart->GetMass() * RollingFrictionCoefficient;
	Acceleration += RollingResistance * DeltaTime;

	// apply acceleration to velocity
	Velocity += Acceleration * DeltaTime;

	if (Kart->HasAuthority())
	{
		ReplicatedState.Velocity = Velocity;
	}
	
}

void UKartMovement::Rotate(const float &DeltaTime, const float& TorqueInput)
{
	// construct quat
	const float ForwardSpeed = FVector::DotProduct(Kart->GetActorForwardVector(), Velocity);
	const float AngleOfRotation = ForwardSpeed / TurnRadius * Torque;
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
		ReplicatedState.Transform = Kart->GetActorTransform();
	}

}

// when server updates move state, propagate to clients
void UKartMovement::OnRep_ReplicatedState()
{
	if (!Kart)
		return;
	
	Kart->SetActorTransform(ReplicatedState.Transform);
	Velocity = ReplicatedState.Velocity;

	// remove all moves included in state
	ClearAckedMoves(ReplicatedState.LastMove);

	// simulate unacked moves
	for (const FKartMoveInput& Move : UnAckedMoves)
	{
		SimulateMoveLocally(Move);
	}
	
}

void UKartMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UKartMovement, ReplicatedState);
	
}

/*
void UKartMovement::Client_OnReceiveMoveState_Implementation()
{

}

bool UKartMovement::Client_OnReceiveMoveState_Validate()
{
	return true;
}
*/

// for use in blueprint speedometer
float UKartMovement::GetSpeed() const
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

void UKartMovement::DrawDebugScreenMessages() const
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


