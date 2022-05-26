// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/PredatorCharacter.h"

// Sets default values
APredatorCharacter::APredatorCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APredatorCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APredatorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APredatorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

