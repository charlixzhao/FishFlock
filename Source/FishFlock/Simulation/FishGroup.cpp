// Fill out your copyright notice in the Description page of Project Settings.


#include "FishGroup.h"
#include "Fish.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"



// Sets default values
AFishGroup::AFishGroup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
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

	ControllerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ControllerMesh"));
	if(!RootComponent) RootComponent = ControllerMesh;
}

TArray<TObjectPtr<AFish>> AFishGroup::GetNearestNeighboursByPercentage(AFish const* InFish, float Percentage)
{
	struct CompareKey
	{
		explicit CompareKey(AFish* F, float D)
			:Distance(D), Fish(F) {;}
		float Distance;
		AFish* Fish;
		bool operator < (CompareKey const& Other) const { return Distance < Other.Distance; }
	};
	TArray<CompareKey> CompareKeys;
	for(AFish* Fish : Fishes)
	{
		CompareKeys.Add(CompareKey(Fish, FVector::Distance(Fish->GetActorLocation(), InFish->GetActorLocation())));
	}
	CompareKeys.Sort();
	int32 const Count = FMath::Max(1, static_cast<int32>(Fishes.Num() * Percentage));
	TArray<TObjectPtr<AFish>> Result;
	for(int32 Idx = 0; Idx < Count; ++Idx)
	{
		Result.Add(CompareKeys[Idx].Fish);
	}
	return Result;
}


void AFishGroup::UpdateFishVelocities_Herd(float DeltaTime)
{
	if(Predator)
	{
		for(AFish* Fish : Fishes)
		{
			Fish->Velocity = (Fish->GetActorLocation() - Predator->GetActorLocation()).GetSafeNormal() * max_speed;
		}
	}
}

// Called when the game starts or when spawned
void AFishGroup::BeginPlay()
{
	Super::BeginPlay();
	Predator = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	
	if(FishClass)
	{
		for(int32 Idx = 0; Idx < FishNum; Idx++)
		{
			AFish* Fish = GetWorld()->SpawnActor<AFish>(FishClass, GetActorLocation(), GetActorRotation());
			if(Fish)
			{
				Fish->BelongingGroup = this;
				Fish->Index = Idx;
				Fishes.Add(Fish);
				NearestNeighbours.Add(Fish, {});
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

	UpdateControlParameters(DeltaTime);
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

	//UpdateFishVelocities_Herd(DeltaTime);
	UpdateFishVelocities_Ball(DeltaTime);
	/*
	for(AFish* Fish : Fishes)
	{
		//dummy placeholder: set the velocity to zero
		//Fish->Velocity = FVector::ZeroVector;

		//Rule 1: Boids try to fly towards the centre of mass of neighbouring boids.
		FVector rule1_vec = Rule_1_Cohesion(Fish);

		//Rule 2: Boids try to keep a small distance away from other objects (including other boids).
		FVector rule2_vec = Rule_2_Separation(Fish);

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
	*/
}

void AFishGroup::UpdateControlParameters(float DeltaTime)
{
	FVector LocationSum = FVector::ZeroVector;
	for(const AFish* Fish : Fishes) LocationSum += Fish->GetActorLocation();
	Centroid = LocationSum / Fishes.Num();
	CentroidToPredatorDistance = FVector::Distance(Centroid, Predator->GetActorLocation());
	

	/*
	TArray<float> Distance;
	for(AFish* Fish : Fishes)
	{
		NearestNeighbours.Add(Fish, GetNearestNeighboursByPercentage(Fish, 0.25f));
		if(NearestNeighbours[Fish].Num() > 0)
		{
			Distance.Add(FVector::Distance(Fish->GetActorLocation(), NearestNeighbours[Fish][0]->GetActorLocation()));
		}
	}
	NearestNeighbourDistance = FMath::Min(Distance);
	*/
	
}


//Rule 1: Boids try to fly towards the centre of mass of neighbouring boids.
FVector AFishGroup::Rule_1_Cohesion(AFish const* Fish)
{
	//Main idea: find the center (average of positions) of nearby boids and move towards it
	FVector center(0);
	//Position of current boid
	FVector const curr_pos = Fish->GetActorLocation();
	//Count of nearby boids, used to calculate average
	int count = 0;
	//Iterate all boids, for those close ones, add to the center
	for (const AFish* i : Fishes)
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
FVector AFishGroup::Rule_2_Separation(AFish const* Fish)
{
	rule_2_dist = Fish->SphereCollider->GetScaledSphereRadius()*2.f;
	//Main idea: move away from near-collision boids
	FVector v(0);
	//Position of current boid
	FVector const curr_pos = Fish->GetActorLocation();
	//Iterate all boids, add a counter velocity to stay away from those close ones
	for (const AFish* i : Fishes)
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
FVector AFishGroup::Rule_3_Alignment(AFish const* Fish)
{
	//Main idea: move towards the same direction (average the velocity) as the nearby boids (fish flock phenomenon)
	FVector v(0);
	//Position of current boid
	FVector const curr_pos = Fish->GetActorLocation();
	//Count of nearby boids, used to calculate average
	int count = 0;
	//Iterate all boids, for those close ones, add to v
	for (const AFish* i : Fishes)
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


void AFishGroup::UpdateFishVelocities_Ball(float DeltaTime)
{
	// Ball Maneuver Rule Scales
	double ball_1_scale = 0.5;
	double ball_2_scale = 1.0;
	double ball_3_scale = 0.1;
	double ball_dist = 100.0;
	for (AFish* Fish : Fishes)
	{
		//dummy placeholder: set the velocity to zero
		//Fish->Velocity = FVector::ZeroVector;

		//Rule 1: Boids try to fly towards the centre of mass of neighbouring boids.
		FVector center = Ball_Get_Center(Fish);
		FVector rule1_vec = FVector::Dist(center, Fish->GetActorLocation()) > ball_dist ? center - Fish->GetActorLocation() : Fish->GetActorLocation() - center;
		rule1_vec.Normalize();

		//Rule 2: Boids try to keep a small distance away from other objects (including other boids).
		FVector rule2_vec = Rule_2_Separation(Fish);

		//Rule 3: Boids try to match velocity with near boids.
		FVector rotate_vec = Ball_Rotate_Arround(Fish, center);


		//Add to old velocity
		Fish->Velocity += ball_1_scale * rule1_vec + ball_2_scale * rule2_vec + ball_3_scale * rotate_vec;
		//Clamp to max speed
		if (Fish->Velocity.Length() > max_speed)
		{
			Fish->Velocity /= Fish->Velocity.Length() / max_speed;
		}
		//UE_LOG(LogTemp, Warning, TEXT("fish vel is %s"), *Fish->Velocity.ToString());
	}
}

FVector AFishGroup::Ball_Get_Center(AFish const* Fish)
{
	//Main idea: find the center (average of positions) of nearby boids and move towards it
	FVector center(0);
	//Position of current boid
	FVector const curr_pos = Fish->GetActorLocation();
	//Count of nearby boids, used to calculate average
	int count = 0;
	//Iterate all boids, for those close ones, add to the center
	for (const AFish* i : Fishes)
	{
		//Check if near
		if (FVector::Dist(i->GetActorLocation(), curr_pos) < rule_1_dist)
		{
			count++;
			center += i->GetActorLocation();
		}
	}
	//Average the result
	center /= count;
	return center;
}

FVector AFishGroup::Ball_Rotate_Arround(AFish const* Fish, FVector& center)
{
	//speed parameters
	double delta_theta = 0.1;
	double delta_phi = 0.01;
	FVector v(0);
	FVector const curr_pos = Fish->GetActorLocation();
	double radius = FVector::Dist(center, curr_pos);
	FVector curr_pos_translated = (curr_pos - center);
	curr_pos_translated.Normalize();
	//To polar coordinate
	double theta = atan2(curr_pos_translated.Y, curr_pos_translated.X);
	double phi = acos(curr_pos_translated.Z);
	v.X = radius * cos(theta + delta_theta) * sin(phi + delta_phi) - radius * cos(theta) * sin(phi);
	v.Y = radius * sin(theta + delta_theta) * sin(phi + delta_phi) - radius * sin(theta) * sin(phi);
	v.Z = radius * cos(phi + delta_phi) - radius * cos(phi);
	return v;
}