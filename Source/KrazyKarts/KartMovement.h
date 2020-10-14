// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KartMovement.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UKartMovement : public UInputComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UKartMovement();

protected:
	
	UPROPERTY(EditAnywhere, Category="Movement")
	float AccelerationRate = 50.f;
	UPROPERTY(EditAnywhere, Category="Movement")
	float MaxSpeed = 1300.f;
	UPROPERTY(EditAnywhere, Category="Movement")
	float TurnRadius = 2.f; // rotation in radians
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
	
	UPROPERTY(Transient)
	class AGoKart* Kart;

	virtual void BeginPlay() override;
	void Rotate(const float &DeltaTime);
	void Accelerate(const float &DeltaTime);
	void Move();

	void AccelerateForward(float AxisInput);
	void RotateVertical(float AxisInput);
	
	UFUNCTION(BlueprintCallable)
	float GetSpeed();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPlayerInput* PlayerInputComponent;
};
