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
	float RotationalAccelerationRate = 0.5f;
	UPROPERTY(EditAnywhere, Category="Movement")
	float MaxSpeed = 1300.f;
	UPROPERTY(EditAnywhere, Category="Movement")
	float MaxAngularSpeed = 90.f; // rotation in degrees per second
	UPROPERTY(EditAnywhere, Category="Movement")
	float Drag = 0.03f;
	
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
