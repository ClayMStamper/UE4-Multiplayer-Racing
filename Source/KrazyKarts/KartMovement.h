// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KartMovement.generated.h"

USTRUCT()
struct FKartMoveInput
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY()
	float Throttle;
	UPROPERTY()
	float Torque;
	UPROPERTY()
	float DeltaTime;
	UPROPERTY()
	float TimeStamp;
};

USTRUCT()
struct FKartMoveState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform Transform;
	UPROPERTY()
	FVector Velocity;
	UPROPERTY()
	FVector Acceleration;
	UPROPERTY()
	FKartMoveInput LastMove;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UKartMovement : public UInputComponent
{
	GENERATED_UCLASS_BODY()

public:
	// Built-in Methods/Events
	virtual bool IsSupportedForNetworking() const override { return true; }
	// Tick: Create a move and send to the server
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	
protected:

	virtual void GetLifetimeReplicatedProps (TArray < FLifetimeProperty > & OutLifetimeProps) const override;

	// Client UObject References
	UPROPERTY(Transient)
	class AGoKart* Kart;
	
	// Client Constant Properties
	UPROPERTY(EditAnywhere, Category="Movement")
	float AccelerationScalar = 50.f;
	UPROPERTY(EditAnywhere, Category="Movement")
	float MaxSpeed = 1300.f;
	UPROPERTY(EditAnywhere, Category="Movement")
	float TurnRadius = 20.f; 
	UPROPERTY(EditAnywhere, Category="Movement")
	float DragCoefficient = 1.f;
	UPROPERTY(EditAnywhere, meta=(UIMin="0.001", UIMax="0.0015"), Category="Movement")
	float RollingFrictionCoefficient = 0.001f;
	
	// Client Variable Props
	UPROPERTY()
	float Throttle;
	UPROPERTY()
	float Torque;
	UPROPERTY()
	FVector Velocity;
	UPROPERTY()
	FVector Acceleration;

	// Client Input-bound Methods
	void Client_AccelerateForward(float AxisInput);
	void Client_RotateYaw(float AxisInput);
	
	// Client methods
	FKartMoveInput CreateMoveInputObject(const float& DeltaTime) const;
	void SimulateMoveLocally(const FKartMoveInput& MoveInput);
	void Rotate(const float &DeltaTime, const float& TorqueInput);
	void Accelerate(const float &DeltaTime, const float& ThrottleInput);
	void UpdateTransform();

	// Server Replicated Properties
	UPROPERTY(ReplicatedUsing=OnRep_ReplicatedMoveState, Transient)
	FKartMoveState ReplicatedMoveState;

	// Client Replication Methods
	UFUNCTION()
    void OnRep_ReplicatedMoveState();

	// Server methods
	UFUNCTION(Server, Reliable, WithValidation)
    void Server_ReceiveMoveInput(const FKartMoveInput& MoveInput);
	
	UFUNCTION(BlueprintCallable)
	float GetSpeed() const;

private:

	static FString RoleEnumToText(ENetRole Role);
	void DrawDebugScreenMessages() const;
	
};
