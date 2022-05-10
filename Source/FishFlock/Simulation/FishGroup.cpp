// Fill out your copyright notice in the Description page of Project Settings.


#include "FishGroup.h"
#include "Fish.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AFishGroup::AFishGroup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFishGroup::BeginPlay()
{
	Super::BeginPlay();

	if(FishClass)
	{
		for(int32 Idx = 0; Idx < FishNum; Idx++)
		{
			AFish* Fish = GetWorld()->SpawnActor<AFish>(FishClass, GetActorLocation(), GetActorRotation());
			if(Fish)
			{
				Fishes.Add(Fish);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Fail To Create Fish!"));	
			}
		}
		
		InitFishPositions();
	}
	
}

// Called every frame
void AFishGroup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateFishVelocities(DeltaTime);
	for(AFish* Fish : Fishes)
	{
		Fish->SetActorLocation(Fish->GetActorLocation() + DeltaTime * Fish->Velocity);
	}

}

void AFishGroup::InitFishPositions()
{
	//TODO: initialize the fish position when the simulation begin, should not create overlap between fishes
	for(AFish* Fish : Fishes)
	{
		//dummy placeholder: just set the location to its current location, which does nothing
		Fish->SetActorLocation(Fish->GetActorLocation());
	}
}

void AFishGroup::UpdateFishVelocities(float DeltaTime)
{
	//TODO: update the fish velocity (expressed in world frame)
	for(AFish* Fish : Fishes)
	{
		//dummy placeholder: set the velocity to zero
		Fish->Velocity = FVector::ZeroVector;
	}
}

