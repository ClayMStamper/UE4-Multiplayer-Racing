// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();
	virtual void Tick(float DeltaTime) override;

	float GetMass();
	UFUNCTION(BlueprintCallable)
    class UKartMovement* GetKartMovementComponent() const;
	
protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(Transient, VisibleAnywhere)
	class UKartMovement* KartMovement;

	UPROPERTY(EditAnywhere)
	float Mass = 1000.f;


};
