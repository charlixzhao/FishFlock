// Fill out your copyright notice in the Description page of Project Settings.


#include "FishGroup.h"
#include "Fish.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "FishFlock/Animation/FishGroupAnimInstance.h"

// Sets default values
AFishGroup::AFishGroup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Controller"));
	if(!RootComponent) RootComponent = Mesh;
	
	//set boids rule scales
	//Cohesion
	rule_1_scale = 0.1;
	rule_1_dist = 999.0;
	//Seperation
	rule_2_scale = 1.0;
	rule_2_dist = 20.0;
	//Alignment
	rule_3_scale = 0.125;
	rule_3_dist = 999.0;
	//maximum speed
	max_speed = 50.0;
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
		FRotator const RotTarget = UKismetMathLibrary::MakeRotFromX(Fish->Velocity);
		Fish->SetActorRotation(UKismetMathLibrary::RInterpTo(Fish->GetActorRotation(), RotTarget, DeltaTime, 5.f));
	}
}

void AFishGroup::InitFishPositions()
{
	//TODO: initialize the fish position when the simulation begin, should not create overlap between fishes
	FRandomStream rand;
	rand.GenerateNewSeed();
	for(AFish* Fish : Fishes)
	{ 
		//dummy placeholder: just set the location to its current location, which does nothing
		Fish->SetActorLocation(Fish->GetActorLocation() + rand.GetUnitVector() * 100.0);
		Fish->Velocity = rand.GetUnitVector() * 10.0;
	}
}

void AFishGroup::UpdateFishVelocities(float DeltaTime)
{
	//TODO: update the fish velocity (expressed in world frame)
	for(AFish* Fish : Fishes)
	{
		//dummy placeholder: set the velocity to zero
		//Fish->Velocity = FVector::ZeroVector;

		//Rule 1: Boids try to fly towards the centre of mass of neighbouring boids.
		FVector rule1_vec = Rule_1_Cohesion(Fish);

		//Rule 2: Boids try to keep a small distance away from other objects (including other boids).
		FVector rule2_vec = Rule_2_Seperation(Fish);

		//Rule 3: Boids try to match velocity with near boids.
		FVector rule3_vec = Rule_3_Alignment(Fish);
		
		
		//Add to old velocity
		Fish->Velocity += rule_1_scale * rule1_vec + rule_2_scale * rule2_vec + rule_3_scale * rule3_vec;
		//Clamp to max speed
		if (Fish->Velocity.Length() > max_speed)
		{
			Fish->Velocity /= Fish->Velocity.Length() / max_speed;
		}
		//UE_LOG(LogTemp, Warning, TEXT("fish vel is %s"), *Fish->Velocity.ToString());
	}
}

FName AFishGroup::GetControllerStateName() const
{
	if(Mesh)
	{
		UFishGroupAnimInstance const* AnimInstance = Cast<UFishGroupAnimInstance>(Mesh->GetAnimInstance());
		if(AnimInstance)
		{
			return AnimInstance->GetCurrentStateName();
		}
	}
	return NAME_None;
}

//Rule 1: Boids try to fly towards the centre of mass of neighbouring boids.
FVector AFishGroup::Rule_1_Cohesion(AFish* Fish)
{
	//Main idea: find the center (average of positions) of nearby boids and move towards it
	FVector center(0);
	//Position of current boid
	FVector curr_pos = Fish->GetActorLocation();
	//Count of nearby boids, used to calculate average
	int count = 0;
	//Iterate all boids, for those close ones, add to the center
	for (AFish* i : Fishes)
	{
		//Check if near
		if (FVector::Dist(i->GetActorLocation(), curr_pos) < rule_1_dist)
		{
			count ++;
			center += i->GetActorLocation();
		}
	}
	//Average the result
	center /= count;
	return center - curr_pos;
}

//Rule 2: Boids try to keep a small distance away from other objects (including other boids).
FVector AFishGroup::Rule_2_Seperation(AFish* Fish)
{
	//Main idea: move away from near-collision boids
	FVector v(0);
	//Position of current boid
	FVector curr_pos = Fish->GetActorLocation();
	//Iterate all boids, add a counter velocity to stay away from those close ones
	for (AFish* i : Fishes)
	{
		//Check if near
		if (FVector::Dist(i->GetActorLocation(), curr_pos) < rule_2_dist)
		{
			v -= (i->GetActorLocation() - curr_pos);
		}
	}
	return v;
}

//Rule 3: Boids try to match velocity with near boids.
FVector AFishGroup::Rule_3_Alignment(AFish* Fish)
{
	//Main idea: move towards the same direction (average the velocity) as the nearby boids (fish flock phenomenon)
	FVector v(0);
	//Position of current boid
	FVector curr_pos = Fish->GetActorLocation();
	//Count of nearby boids, used to calculate average
	int count = 0;
	//Iterate all boids, for those close ones, add to v
	for (AFish* i : Fishes)
	{
		//Check if near
		if (FVector::Dist(i->GetActorLocation(), curr_pos) < rule_3_dist)
		{
			count++;
			v += i->Velocity;
		}
	}
	//Average the result
	v /= count;
	return v;
}

//Find the center position of fish school
FVector AFishGroup::Find_Centroid()
{
	FVector Centroid_Pos(0);

	int Count = 0;
	for (AFish* Fish : Fishes)
	{
		Count ++;
		Centroid_Pos += Fish->GetActorLocation();
	}
	//Average the result
	Centroid_Pos /= Count;
	return Centroid_Pos;
}

//Predator_Attack: Herd -> Vacuole
FVector Rule_Vacuole()
{
	FVector v(0);

	return v;
}

//Predator_Attack: School || Compact -> Flash
FVector Rule_Flash(AFish* Prey, AFish* Predator)
{
	FVector v(0);

	return v;
}
