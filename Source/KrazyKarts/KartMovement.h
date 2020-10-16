// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KartMovement.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UKartMovement : public UInputComponent
{
	GENERATED_UCLASS_BODY()

public:
	// Component Replication props
	UPROPERTY(Replicated)
	uint32 bReplicatedFlag:1;
	virtual bool IsSupportedForNetworking() const override { return true; }
	
	// Sets default values for this component's properties
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPlayerInput* PlayerInputComponent;
	void Client_AccelerateForward(float AxisInput);
	void Client_RotateYaw(float AxisInput);
protected:
	
	UPROPERTY(EditAnywhere, Category="Movement")
	float AccelerationRate = 50.f;
	UPROPERTY(EditAnywhere, Category="Movement")
	float MaxSpeed = 1300.f;
	UPROPERTY(EditAnywhere, Category="Movement")
	float TurnRadius = 20.f; 
	UPROPERTY(EditAnywhere, Category="Movement")
	float DragCoefficient = 1.f;
	UPROPERTY(EditAnywhere, meta=(UIMin="0.001", UIMax="0.0015"), Category="Movement")
	float RollingFrictionCoefficient = 0.001f;
	
	UPROPERTY(Transient)
	FVector Velocity;
	UPROPERTY(Transient)
	FVector Acceleration;
	UPROPERTY(Transient)
	float MagTorque;
	UPROPERTY(ReplicatedUsing=OnRep_Transform)
	FTransform ReplicatedTransform;
	
	UPROPERTY(Transient)
	class AGoKart* Kart;

	virtual void BeginPlay() override;
	void Rotate(const float &DeltaTime);
	void Accelerate(const float &DeltaTime);
	void Move();

	UFUNCTION()
	void OnRep_Transform() const;
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_AccelerateForward(float AxisInput);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RotateYaw(float AxisInput); // rotate along vertical axis
	virtual void GetLifetimeReplicatedProps (TArray < FLifetimeProperty > & OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable)
	float GetSpeed();

private:

	FString RoleEnumToText(ENetRole Role);

	void DrawDebugScreenMessages();
	
};
