// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "KartMovement.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	KartMovement = FindComponentByClass<UKartMovement>();
	
}

float AGoKart::GetMass()
{
	return Mass;
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UKartMovement* AGoKart::GetKartMovementComponent() const
{
	return KartMovement;
}



