// Fill out your copyright notice in the Description page of Project Settings.


#include "FishGroup.h"
#include "Fish.h"
#include "VectorTypes.h"
#include "Animation/FishControllerAnimInstance.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetArrayLibrary.h"
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
	rule_2_dist = 60.0;
	//Alignment
	rule_3_scale = 0.125;
	rule_3_dist = 999.0;
	//maximum speed
	max_speed_current = 50.0;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	if(!RootComponent) RootComponent = SceneRoot;
	
	ControllerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ControllerMesh"));
	ControllerMesh->SetupAttachment(RootComponent);

	TravelSpline = CreateDefaultSubobject<USplineComponent>(TEXT("TravelSpline"));
	TravelSpline->SetupAttachment(RootComponent);
}

bool AFishGroup::DoesAnyFishVision() const
{
	for(const AFish* Fish : Fishes)
	{
		if(Fish->bVision) return true;
	}
	return false;
}

bool AFishGroup::DoesAnyFishFeelRipple() const
{
	for(const AFish* Fish : Fishes)
	{
		if(Fish->bFeelRipple) return true;
	}
	return false;
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

TArray<FVector> AFishGroup::DetectNoCollisionDirections(AFish* Fish) const
{
	TArray<FVector> Results;
	int32 MaxPointsNum = 10;
	FVector Forward = UKismetMathLibrary::GetForwardVector(Fish->GetActorRotation());
	float R = Fish->VisionDistance;
	int32 Iteration = 6;
	float Interval = R / static_cast<float>(Iteration);
	for(float Distance = 0.f; Distance <= R; Distance += Interval)
	{
		float n = R-Distance;
		float r = FMath::Sqrt(R*R-n*n);
		FVector const Base = (Forward.Cross(FVector(0.f, 0.f,1.f))).GetSafeNormal();
		float DistanceFraction = Distance/R;
		int32 PointsNum = FMath::Max(1, static_cast<int32>(DistanceFraction * MaxPointsNum));
		for(int32 Idx = 0; Idx <= PointsNum; ++Idx)
		{
			float Angle = (360.f) * (static_cast<float>(Idx)/static_cast<float>(PointsNum));
			FRotator Rotation = UKismetMathLibrary::RotatorFromAxisAndAngle(Forward, Angle);
			FVector Outward = Rotation.RotateVector(Base);
			FVector SamplePoint = Fish->GetActorLocation() + Forward*n + Outward*r;
			FHitResult OutHit;
			bool bDetectCollision = UKismetSystemLibrary::LineTraceSingle(this,
				Fish->GetActorLocation(), SamplePoint, UEngineTypes::ConvertToTraceType(ECC_Visibility),
				true, {},
				bDrawCollisionDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
				OutHit, true);
			if(!bDetectCollision)
			{
				Results.Push((SamplePoint-Fish->GetActorLocation()).GetSafeNormal());
			}
		}
	}
	return Results;
}

float AngleBetweenVectors(FVector V, FVector W)
{
	V.Normalize();
	W.Normalize();
	return FMath::RadiansToDegrees(FMath::Acos(V.Dot(W)));
}

//find the index and value in a array which has smallest distance
// to the Goal, distance computing by the dist func
template<typename T, typename Dist>
auto FindNearest(TArray<T> const& Values, T const& Goal, Dist DistFunc)
{
	ensure(Values.Num() > 0);
	using DistType = std::invoke_result_t<Dist, T const&, T const&>;
	
	int32 ArgMin = -1;
	DistType CurrentMin{};
	for(int32 Idx = 0; Idx < Values.Num(); ++Idx)
	{
		const float Distance = DistFunc(Values[Idx], Goal);
		if(ArgMin == -1 || Distance < CurrentMin)
		{
			CurrentMin = Distance;
			ArgMin = Idx;
		}
	}
	return TTuple<int32, DistType>{ArgMin, CurrentMin};
}

void AFishGroup::UpdateFishVelocities_Wander(float DeltaTime)
{
	max_speed_current = max_speed_normal;
	FVector SplineTargetCenter;
	for(int32 Idx = 0; Idx < Fishes.Num(); ++Idx)
	{
		AFish* Fish = Fishes[Idx];
		FVector TargetVelocity;
		if(Leaders.Contains(Fish))
		{
			Fish->CumulativeWanderDistance += max_speed_current * DeltaTime/2.f;
			Fish->CumulativeWanderDistance = FMath::Fmod(Fish->CumulativeWanderDistance, 	TravelSpline->GetSplineLength());
			const FVector LeaderLocation = TravelSpline->GetWorldLocationAtDistanceAlongSpline(Fish->CumulativeWanderDistance);
			SplineTargetCenter += LeaderLocation;
			TargetVelocity = (LeaderLocation - Fish->GetActorLocation()).GetSafeNormal() * max_speed_current;
		}
		else
		{
			FVector FollowLeaders = Rule_FollowLeaders(Fish);
			FVector Separation = Rule_2_Separation(Fish);
			FVector Alignment = Rule_3_Alignment(Fish);
			TargetVelocity += rule_1_scale * FollowLeaders + rule_2_scale * Separation + rule_3_scale * Alignment;
			TargetVelocity = TargetVelocity.GetSafeNormal() * max_speed_current;
		}

		const TArray<FVector> NoCollisionDirections = DetectNoCollisionDirections(Fish);
		if(NoCollisionDirections.Num() == 0)
		{
			Fish->Velocity = FVector::ZeroVector;
			return;
		}
		else
		{
			//find the possible direction which smallest angle with the current velocity
			float NearestValue;
			int32 ArgNearestDirection;
			Tie(ArgNearestDirection, NearestValue) = FindNearest(NoCollisionDirections, TargetVelocity, AngleBetweenVectors);
			const FVector NearestDirection = NoCollisionDirections[ArgNearestDirection].GetSafeNormal() * max_speed_current;
			if(NearestValue >= 5.f)
			{
				Fish->Velocity = FMath::VInterpTo(Fish->Velocity, NearestDirection, DeltaTime, 5.f);
			}
			else
			{
				Fish->Velocity = FMath::VInterpTo(Fish->Velocity, TargetVelocity, DeltaTime, 5.f);
			}
		}
	}
	DrawDebugSphere(GetWorld(), SplineTargetCenter/Leaders.Num(), 15.f, 10, FColor::Green);
}


void AFishGroup::UpdateFishVelocities_Skitter(float DeltaTime)
{
	max_speed_current = max_speed_normal;
}

void AFishGroup::UpdateFishVelocities_Compact(float DeltaTime)
{
	max_speed_current = max_speed_normal;
	for(int32 Idx = 0; Idx < Fishes.Num(); ++Idx)
	{
		AFish* Fish = Fishes[Idx];
		FVector TargetVelocity;
		if(Leaders.Contains(Fish))
		{
			Fish->CumulativeWanderDistance += max_speed_current * DeltaTime;
			Fish->CumulativeWanderDistance = FMath::Fmod(Fish->CumulativeWanderDistance, 	TravelSpline->GetSplineLength());
			const FVector LeaderLocation = TravelSpline->GetWorldLocationAtDistanceAlongSpline(Fish->CumulativeWanderDistance);
			TargetVelocity = (LeaderLocation - Fish->GetActorLocation()).GetSafeNormal() * max_speed_current;
		}
		else
		{
			FVector Cohesion = Rule_1_Cohesion(Fish);
			FVector Separation = Rule_2_Separation(Fish);
			FVector Alignment = Rule_3_Alignment(Fish);
			TargetVelocity += rule_1_scale*2 * Cohesion + rule_2_scale * Separation + rule_3_scale * Alignment;
			TargetVelocity = TargetVelocity.GetSafeNormal() * max_speed_current;
		}

		const TArray<FVector> NoCollisionDirections = DetectNoCollisionDirections(Fish);
		if(NoCollisionDirections.Num() == 0)
		{
			Fish->Velocity = FVector::ZeroVector;
			return;
		}
		else
		{
			//find the possible direction which smallest angle with the current velocity
			float NearestValue;
			int32 ArgNearestDirection;
			Tie(ArgNearestDirection, NearestValue) = FindNearest(NoCollisionDirections, TargetVelocity, AngleBetweenVectors);
			const FVector NearestDirection = NoCollisionDirections[ArgNearestDirection].GetSafeNormal() * max_speed_current;
			if(NearestValue >= 5.f)
			{
				Fish->Velocity = FMath::VInterpTo(Fish->Velocity, NearestDirection, DeltaTime, 5.f);
			}
			else
			{
				Fish->Velocity = FMath::VInterpTo(Fish->Velocity, TargetVelocity, DeltaTime, 5.f);
			}
		}
	}
}


void AFishGroup::UpdateFishVelocities_Herd(float DeltaTime)
{
	max_speed_current = max_speed_fast_avoid;
	if(Predator)
	{
		const FVector DirectionToPredator = Centroid - Predator->GetActorLocation();
		for(AFish* Fish : Fishes)
		{
			Fish->Velocity = DirectionToPredator.GetSafeNormal() * max_speed_fast_avoid;
		}
	}
}

void AFishGroup::UpdateFishVelocities_Avoid(float DeltaTime)
{
	max_speed_current = max_speed_normal;
	if(Predator)
	{
		for(AFish* Fish : Fishes)
		{
			Fish->Velocity = (Fish->GetActorLocation() - Predator->GetActorLocation()).GetSafeNormal() * max_speed_current;
		}
	}
}

void AFishGroup::UpdateFishVelocities_FastAvoid(float DeltaTime)
{
	max_speed_current = max_speed_fast_avoid;
	if(Predator)
	{
		for(AFish* Fish : Fishes)
		{
			Fish->Velocity = (Fish->GetActorLocation() - Predator->GetActorLocation()).GetSafeNormal() * max_speed_fast_avoid;
		}
	}
}

void AFishGroup::EnterFastAvoid()
{
	max_speed_current = max_speed_fast_avoid;
}

void AFishGroup::LeaveFastAvoid()
{
	max_speed_current = max_speed_normal;
}

void AFishGroup::EnterSkitter()
{
	max_speed_current = max_speed_normal;
	for(const auto Fish : Fishes)
	{
		Fish->Velocity = FMath::VRand() * max_speed_current;
	}
}

// Called when the game starts or when spawned
void AFishGroup::BeginPlay()
{
	Super::BeginPlay();
	max_speed_current = max_speed_normal;
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

		RandomChooseLeaders();
		InitFishPositions();
	}

	//UE_LOG(LogTemp, Warning, TEXT("fish num is %d"), Fishes.Num());
}

// Called every frame
void AFishGroup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateFishVelocities(DeltaTime);
	for(AFish* Fish : Fishes)
	{
		Fish->Velocity += DeltaTime * Fish->Acceleration;
		Fish->SetActorLocation(Fish->GetActorLocation() + DeltaTime * Fish->Velocity);
		FRotator const RotTarget = UKismetMathLibrary::MakeRotFromX(Fish->Velocity);
		Fish->SetActorRotation(UKismetMathLibrary::RInterpTo(Fish->GetActorRotation(), RotTarget, DeltaTime, 5.f));
	}

	UpdateControlParameters(DeltaTime);
}

void AFishGroup::InitFishPositions()
{
	const FVector SplineOrigin = TravelSpline->GetLocationAtTime(0.f, ESplineCoordinateSpace::World);
	FRandomStream rand;
	rand.GenerateNewSeed();
	for(int32 Idx = 0; Idx < Fishes.Num(); ++Idx)
	{
		AFish* Fish = Fishes[Idx];
		if(Leaders.Contains(Fish))
		{
			const float SplineDistance = FMath::RandRange(0.f, 50.f);
			Fish->SetActorLocation(TravelSpline->GetWorldLocationAtDistanceAlongSpline(SplineDistance));
			Fish->CumulativeWanderDistance = SplineDistance;
		}
		else
		{
			Fish->SetActorLocation(SplineOrigin + rand.GetUnitVector() * 300.0);
			Fish->Velocity = rand.GetUnitVector() * 10.0;
		}
	}
}

void AFishGroup::RandomChooseLeaders()
{
	const int32 LeaderNum_Clamped = FMath::Clamp(LeaderNum, 0, FishNum);
	Leaders.Empty();
	auto FishesShuffled = Fishes;
	if (FishesShuffled.Num() > 0)
	{
		const int32 LastIndex = FishesShuffled.Num() - 1;
		for (int32 i = 0; i <= LastIndex; ++i)
		{
			const int32 Index = FMath::RandRange(i, LastIndex);
			if (i != Index)
			{
				FishesShuffled.Swap(i, Index);
			}
		}
	}
	for(int32 Idx = 0; Idx < LeaderNum_Clamped; ++Idx)
	{
		Leaders.Add(FishesShuffled[Idx]);
	}
}

void AFishGroup::UpdateFishVelocities(float DeltaTime)
{
	//TODO: update the fish velocity (expressed in world frame)
	FName const CurrentStateName = Cast<UFishControllerAnimInstance>(ControllerMesh->GetAnimInstance())->GetCurrentStateName();
	if(CurrentStateName == "Wander") UpdateFishVelocities_Wander(DeltaTime);
	else if (CurrentStateName == "Compact") UpdateFishVelocities_Compact(DeltaTime);
	else if (CurrentStateName == "Ball") UpdateFishVelocities_Ball(DeltaTime);
	else if (CurrentStateName == "Avoid") UpdateFishVelocities_Avoid(DeltaTime);
	else if (CurrentStateName == "FastAvoid") UpdateFishVelocities_FastAvoid(DeltaTime);
	else if (CurrentStateName == "Skitter") UpdateFishVelocities_Skitter(DeltaTime);
	else if (CurrentStateName == "Herd") UpdateFishVelocities_Herd(DeltaTime);
	else if (CurrentStateName == "FlashOut") UpdateFishVelocities_FlashOutward(DeltaTime);
	else if (CurrentStateName == "FlashIn") UpdateFishVelocities_FlashInward(DeltaTime);
	else if (CurrentStateName == "Split") UpdateFishVelocities_Split(DeltaTime);
	else if (CurrentStateName == "Fountain") UpdateFishVelocities_Fountain(DeltaTime);
	else if (CurrentStateName == "Hourglass") UpdateFishVelocities_Hourglass(DeltaTime);
	else if (CurrentStateName == "Join") UpdateFishVelocities_Join(DeltaTime);
	else
	{
		ensureMsgf(false, TEXT("enter a state which is bound to any updating function"));
	}
}

void AFishGroup::UpdateControlParameters(float DeltaTime)
{
	FVector LocationSum = FVector::ZeroVector;
	for(const AFish* Fish : Fishes) LocationSum += Fish->GetActorLocation();
	Centroid = LocationSum / Fishes.Num();
	CentroidToPredatorDistance = FVector::Distance(Centroid, Predator->GetActorLocation());
	TArray<float> Distance;
	for(auto const Fish:Fishes) Distance.Add(FVector::Distance(Fish->GetActorLocation(), Predator->GetActorLocation()));
	NearestToPredatorDistance = FMath::Min(Distance);

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

FVector AFishGroup::Rule_FollowLeaders(AFish const* Fish)
{
	FVector Center(0);
	for(const auto Leader : Leaders)
	{
		Center += Leader->GetActorLocation();
	}
	return Center/Leaders.Num() - Fish->GetActorLocation();
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
	//rule_2_dist = Fish->SphereCollider->GetScaledSphereRadius()*4.f;
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

FVector AFishGroup::Rule_2_Compact(AFish const* Fish)
{
	const double rule_2_compact_dist = 20.0;
	//rule_2_dist = Fish->SphereCollider->GetScaledSphereRadius()*4.f;
	//Main idea: move away from near-collision boids
	FVector v(0);
	//Position of current boid
	FVector const curr_pos = Fish->GetActorLocation();
	//Iterate all boids, add a counter velocity to stay away from those close ones
	for (const AFish* i : Fishes)
	{
		//Check if near
		if (FVector::Dist(i->GetActorLocation(), curr_pos) < rule_2_compact_dist)
		{
			v -= (i->GetActorLocation() - curr_pos);
		}
	}
	return v;
}


void AFishGroup::UpdateFishVelocities_Ball(float DeltaTime)
{
	// Ball Maneuver Rule Scales
	const double ball_1_scale = 0.5;
	const double ball_2_scale = 1.0;
	const double ball_3_scale = 0.1;
	const double ball_dist = 100.0;
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
		FVector rotate_vec = Ball_Rotate_Around(Fish, center);


		//Add to old velocity
		Fish->Velocity += ball_1_scale * rule1_vec + ball_2_scale * rule2_vec + ball_3_scale * rotate_vec;
		//Clamp to max speed
		if (Fish->Velocity.Length() > max_speed_current)
		{
			Fish->Velocity /= Fish->Velocity.Length() / max_speed_current;
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

FVector AFishGroup::Ball_Rotate_Around(AFish const* Fish, const FVector& center)
{
	//speed parameters
	const double delta_theta = 0.1;
	const double delta_phi = 0.01;
	FVector v(0);
	FVector const curr_pos = Fish->GetActorLocation();
	const double radius = FVector::Dist(center, curr_pos);
	FVector curr_pos_translated = (curr_pos - center);
	curr_pos_translated.Normalize();
	//To polar coordinate
	const double theta = atan2(curr_pos_translated.Y, curr_pos_translated.X);
	const double phi = acos(curr_pos_translated.Z);
	v.X = radius * cos(theta + delta_theta) * sin(phi + delta_phi) - radius * cos(theta) * sin(phi);
	v.Y = radius * sin(theta + delta_theta) * sin(phi + delta_phi) - radius * sin(theta) * sin(phi);
	v.Z = radius * cos(phi + delta_phi) - radius * cos(phi);
	return v;
}

void AFishGroup::UpdateFishVelocities_FlashOutward(float DeltaTime)
{
	if(Predator)
	{
		for(AFish* Fish : Fishes)
		{
			Fish->Velocity = (Fish->GetActorLocation() - Predator->GetActorLocation()).GetSafeNormal() * 1000.f;
			if (Fish->Velocity.Length() > 1000.f)
			{
				Fish->Velocity /= Fish->Velocity.Length() / 1000.f;
			}
		}
	}
}

void AFishGroup::UpdateFishVelocities_FlashInward(float DeltaTime)
{
	if(Predator)
	{
		if(Flash_Initialized == false)
		{
			for (const AFish* i : Fishes)
			{
				Flash_Return_Position += i->GetActorLocation();
			}
			
			Flash_Return_Position /= Fishes.Num();
			Flash_Initialized = true;
		}
		for(AFish* Fish : Fishes)
		{
			Fish->Velocity = (Flash_Return_Position - Fish->GetActorLocation()).GetSafeNormal() * 700.f;
			if (Fish->Velocity.Length() > 700.f)
			{
				Fish->Velocity /= Fish->Velocity.Length() / 700.f;
			}
		}
	}
}

void AFishGroup::UpdateFishVelocities_Split(float DeltaTime)
{
	if(!Split_Initialized)
	{
		Split_and_FindLeader();
	}
	
	//Set leader position as center for cohesion rule
	if(LeftLeader)
	{
		const FVector LeftCenter = LeftLeader->GetActorLocation();
			
		for (AFish * Fish : SplitLeft)
		{
			if(Fish == LeftLeader)
			{
				if(LeftLeaderEscapeDirection.Equals(FVector(0,0,0)))
				{
					const FVector A = Predator->GetVelocity();
					Fish->Velocity = (FVector(-A.Y,A.X,0)).GetSafeNormal() * max_speed_escape;
					/*const FVector B = (FVector(-A.Y,A.X,0)).GetSafeNormal() * max_speed_escape;
					Fish->Velocity = (A.GetSafeNormal() * max_speed_escape) * 0.5 + B * 0.5;*/
				}
				else
				{
					Fish->Velocity /= Fish->Velocity.Length() / max_speed_escape;
				}
			}
			else
			{
				const double Cohension_Scale = 2.0;
				const double Separation_Scale = 4.0;
				const double Alignment_Scale = 0.5;

				FVector Cohesion_Vec = LeftCenter - Fish->GetActorLocation();
				FVector Separation_Vec = Rule_2_Separation(Fish);
				FVector Alignment_Vec = Rule_3_Alignment(Fish);

				Fish->Velocity = (Cohension_Scale * Cohesion_Vec) + (Separation_Scale * Separation_Vec) + (Alignment_Scale * Alignment_Vec);
			}
		}
	}
	if(RightLeader)
	{
		const FVector RightCenter = RightLeader->GetActorLocation();

		
		for (AFish * Fish : SplitRight)
		{
			if(Fish == RightLeader)
			{
				if(RightLeaderEscapeDirection.Equals(FVector(0,0,0)))
				{
					const FVector A = Predator->GetVelocity();
					Fish->Velocity = (FVector(A.Y,-A.X,0)).GetSafeNormal() * max_speed_escape;
					/*const FVector B = (FVector(A.Y,-A.X,0)).GetSafeNormal() * max_speed_escape;
					Fish->Velocity = (A.GetSafeNormal() * max_speed_escape) * 0.5 + B * 0.5;*/
				}
				else
				{
					Fish->Velocity /= Fish->Velocity.Length() / max_speed_escape;
				}
				
			}
			else
			{
				const double Cohension_Scale = 2.0;
				const double Separation_Scale = 4.0;
				const double Alignment_Scale = 0.5;

				FVector Cohesion_Vec = RightCenter - Fish->GetActorLocation();
				FVector Separation_Vec = Rule_2_Separation(Fish);
				FVector Alignment_Vec = Rule_3_Alignment(Fish);

				Fish->Velocity = (Cohension_Scale * Cohesion_Vec) + (Separation_Scale * Separation_Vec) + (Alignment_Scale * Alignment_Vec);
			}
		}
	}
}

void AFishGroup::Split_and_FindLeader()
{
	float LeftMaxDist = FLT_MIN;
	float RightMaxDist = FLT_MIN;
	
	for(AFish* Fish: Fishes)
	{
		// Calculate the Cosine between (prey - predator)vector and the X axis, split fishes into two groups
		const FVector A = Predator->GetVelocity();
		const FVector B = Fish->GetActorLocation() - Predator->GetActorLocation();
		if ((A.X * B.Y - B.X * A.Y) > 0)
		{
			SplitLeft.Add(Fish);
			//UE_LOG(LogTemp, Warning, TEXT("Left side adds one more fish."));
			const float TempDist = FVector::Dist(Fish->GetActorLocation(),Predator->GetActorLocation());
			if(TempDist > LeftMaxDist)
			{
				LeftMaxDist = TempDist;
				LeftLeader = Fish;
			}
		}
		else
		{
			SplitRight.Add(Fish);
			//UE_LOG(LogTemp, Warning, TEXT("Right side adds one more fish."));
			const float TempDist = FVector::Dist(Fish->GetActorLocation(),Predator->GetActorLocation());
			if(TempDist > RightMaxDist)
			{
				RightMaxDist = TempDist;
				RightLeader = Fish;
			}
		}
	}
	Split_Initialized = true;
}

void AFishGroup::UpdateFishVelocities_Fountain(float DeltaTime)
{
	const FVector Predator_Velocity = Predator->GetVelocity();
	const FVector Predator_Location = Predator->GetActorLocation();
	const double Velocity_Scale = 0.66;
	const double Rotation_Scale = 0.34;
	for(AFish* Fish : Fishes)
	{
		//front
		if((Fish->GetActorLocation() - Predator_Location).CosineAngle2D(FVector(1,0,0)) >= -0.5)
		{
			const FVector A = Predator->GetVelocity();
			const FVector B = Fish->GetActorLocation() - Predator->GetActorLocation();
			//UE_LOG(LogTemp, Warning, TEXT("Predator Velocity: %f, %f, %f"), A.X,A.Y,A.Z);
			
			//Left
			if ((A.X * B.Y - B.X * A.Y) < 0)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Left side adds one fish."));
				
				const FVector Runaway_Velocity = (Fish->GetActorLocation() - Predator->GetActorLocation()).GetSafeNormal();
				//const FVector Rotational_Velocity = Predator_Velocity.GetSafeNormal() * -1;
				FVector New_Velocity(Runaway_Velocity.Y,-Runaway_Velocity.X,Runaway_Velocity.Z);
				Fish->Velocity = New_Velocity * max_speed_escape;
				//Fish->Velocity = (Velocity_Scale * Runaway_Velocity + Rotational_Velocity * Rotation_Scale) * max_speed_escape;
				Fish->Velocity.Z = 0;
				/*if(abs(Fish->Velocity.Z) > 5)
				{
					Fish->Velocity.Z /= Fish->Velocity.Z / 5; 
				}*/
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("Right side adds one fish."));
				
				const FVector Runaway_Velocity = (Fish->GetActorLocation() - Predator->GetActorLocation()).GetSafeNormal();
				FVector New_Velocity(-Runaway_Velocity.Y,Runaway_Velocity.X,Runaway_Velocity.Z);
				
				Fish->Velocity = New_Velocity * max_speed_escape;
				Fish->Velocity.Z = 0;
				/*if(abs(Fish->Velocity.Z) > 5)
				{
					Fish->Velocity.Z /= Fish->Velocity.Z / 5; 
				}*/
			}
		}
		else
		{
			const FVector Cohesion_Vec = Rule_1_Cohesion(Fish);
			const FVector Separation_Vec = Rule_2_Separation(Fish);
			const FVector Alignment_Vec = Rule_3_Alignment(Fish);
			const double Cohesion_Scale = 0.2;
			const double Separation_Scale = 1.0;
			const double Alignment_Scale = 0.5;
			Fish->Velocity += Cohesion_Vec * Cohesion_Scale + Separation_Vec * Separation_Scale + Alignment_Vec * Alignment_Scale;
			
			//Clamp to max speed
			if (Fish->Velocity.Length() > max_speed_escape)
			{
				Fish->Velocity /= Fish->Velocity.Length() / max_speed_escape;
			}
		}
	}
}

void AFishGroup::UpdateFishVelocities_Hourglass(float DeltaTime)
{
	FVector Average_Velocity(0,0,0);
	//get fish average velocity
	for(const AFish* Fish:Fishes)
	{
		Average_Velocity += Fish->Velocity;
	}
	Average_Velocity = Average_Velocity / Fishes.Num();
	if(!Hourglass_Initialized)
	{
		Hourglass_Initial_Vector = Average_Velocity;
		Hourglass_Initialized = true;
	}

	TArray<TObjectPtr<AFish>> Hourglass_Left;
	TArray<TObjectPtr<AFish>> Hourglass_Right;

	for(AFish *Fish: Fishes)
	{
		const FVector A = Predator->GetVelocity();
		const FVector B = Fish->GetActorLocation() - Predator->GetActorLocation();
		//Left
		if ((A.X * B.Y - B.X * A.Y) < 0)
		{
			Hourglass_Left.Add(Fish);
		}
		else
		{
			Hourglass_Right.Add(Fish);
		}
	}

	//initialize turning direction only if it has not been initialized
	if(Hourglass_Turning_Direction == 0)
	{
		if(Hourglass_Left.Num() > Hourglass_Right.Num())
		{
			Hourglass_Turning_Direction = 1;
		}
		else Hourglass_Turning_Direction = 2;
	}
	
	//Orthogonal Velocity, turning clockwise 90 degrees
	const double Turning_Scale = 0.04;
	
	/*FVector Turning_Velocity(0,0,0);
	Turning_Velocity.X = Average_Velocity.Y * -1;
	Turning_Velocity.Y = Average_Velocity.X;
	Turning_Velocity.Z = 0;*/

	FVector Turning_Velocity(0,0,0);
	if(Hourglass_Turning_Direction == 2)
	{
		Turning_Velocity.X = -Average_Velocity.Y;
		Turning_Velocity.Y = Average_Velocity.X;
		Turning_Velocity.Z = 0;
	}
	else
	{
		Turning_Velocity.X = Average_Velocity.Y;
		Turning_Velocity.Y = -Average_Velocity.X;
		Turning_Velocity.Z = 0;
	}

	if(Hourglass_Left.Num() > 0)
	{
		for(AFish *Fish: Hourglass_Left)
		{
			//left fish, if turn left
			if(Hourglass_Turning_Direction == 1)
			{
				if(HasFlockShiftNinetyDegree() == false)
				{
					Fish->Velocity += Turning_Velocity * Turning_Scale;
				}
				Fish->Velocity /= Fish->Velocity.Length() / 900.f;
		
				//Clamp to max speed
				if (Fish->Velocity.Length() > 900.f)
				{
					Fish->Velocity /= Fish->Velocity.Length() / 900.f;
				}
			}
			else
			{
				if(HasFlockShiftNinetyDegree() == false)
				{
					Fish->Velocity += Turning_Velocity * Turning_Scale;
				}
				Fish->Velocity /= Fish->Velocity.Length() / 1500.f;
		
				//Clamp to max speed
				if (Fish->Velocity.Length() > 1500.f)
				{
					Fish->Velocity /= Fish->Velocity.Length() / 1500.f;
				}
			}
		}
	}
	//right fishes
	if(Hourglass_Right.Num() > 0)
	{
		for(AFish *Fish: Hourglass_Right)
		{
			//right fish, if turn right
			if(Hourglass_Turning_Direction == 2)
			{
				if(HasFlockShiftNinetyDegree() == false)
				{
					Fish->Velocity += Turning_Velocity * Turning_Scale;
				}
				Fish->Velocity /= Fish->Velocity.Length() / 900.f;
		
				//Clamp to max speed
				if (Fish->Velocity.Length() > 900.f)
				{
					Fish->Velocity /= Fish->Velocity.Length() / 900.f;
				}
			}
			else
			{
				if(HasFlockShiftNinetyDegree() == false)
				{
					Fish->Velocity += Turning_Velocity * Turning_Scale;
				}
				Fish->Velocity /= Fish->Velocity.Length() / 1500.f;
		
				//Clamp to max speed
				if (Fish->Velocity.Length() > 1500.f)
				{
					Fish->Velocity /= Fish->Velocity.Length() / 1500.f;
				}
			}
		}
	}
	FVector temp_current_average(0,0,0);
	for(AFish *Fish:Fishes)
	{
		temp_current_average += Fish->Velocity;
	}
	Hourglass_Current_Vector = temp_current_average / Fishes.Num();
}

void AFishGroup::UpdateFishVelocities_Join(float DeltaTime)
{
	max_speed_current = max_speed_normal;
	for(int32 Idx = 0; Idx < Fishes.Num(); ++Idx)
	{
		AFish* Fish = Fishes[Idx];
		FVector TargetVelocity;
		if(Leaders.Contains(Fish))
		{
			Fish->CumulativeWanderDistance += max_speed_current * DeltaTime;
			Fish->CumulativeWanderDistance = FMath::Fmod(Fish->CumulativeWanderDistance, 	TravelSpline->GetSplineLength());
			const FVector LeaderLocation = TravelSpline->GetWorldLocationAtDistanceAlongSpline(Fish->CumulativeWanderDistance);
			TargetVelocity = (LeaderLocation - Fish->GetActorLocation()).GetSafeNormal() * max_speed_current;
		}
		else
		{
			FVector Cohesion = Rule_1_Cohesion(Fish);
			FVector Separation = Rule_2_Separation(Fish);
			FVector Alignment = Rule_3_Alignment(Fish);
			TargetVelocity += rule_1_scale*10 * Cohesion + rule_2_scale * Separation + rule_3_scale * Alignment;
			TargetVelocity = TargetVelocity.GetSafeNormal() * max_speed_current;
		}

		const TArray<FVector> NoCollisionDirections = DetectNoCollisionDirections(Fish);
		if(NoCollisionDirections.Num() == 0)
		{
			Fish->Velocity = FVector::ZeroVector;
			return;
		}
		else
		{
			//find the possible direction which smallest angle with the current velocity
			float NearestValue;
			int32 ArgNearestDirection;
			Tie(ArgNearestDirection, NearestValue) = FindNearest(NoCollisionDirections, TargetVelocity, AngleBetweenVectors);
			const FVector NearestDirection = NoCollisionDirections[ArgNearestDirection].GetSafeNormal() * max_speed_current;
			if(NearestValue >= 5.f)
			{
				Fish->Velocity = FMath::VInterpTo(Fish->Velocity, NearestDirection, DeltaTime, 5.f);
			}
			else
			{
				Fish->Velocity = FMath::VInterpTo(Fish->Velocity, TargetVelocity, DeltaTime, 5.f);
			}
		}
	}
	
	for(AFish *Fish:Fishes)
	{
		AverageDistanceFromCentroid += (Fish->GetActorLocation() - Centroid).Length();
	}
	AverageDistanceFromCentroid /= Fishes.Num();
}

bool AFishGroup::IsAnyFishFront()
{
	const FVector Predator_Location = Predator->GetActorLocation();
	for(const AFish* Fish : Fishes)
	{
		if((Fish->GetActorLocation() - Predator_Location).CosineAngle2D(FVector(1,0,0)) >= -0.5)
		{
			return true;
		}
	}
	return false;
}

bool AFishGroup::HasFlockShiftNinetyDegree()
{
	if(Hourglass_Initialized)
	{
		if((Hourglass_Initial_Vector.CosineAngle2D(Hourglass_Current_Vector)) < 0)
		{
			return true;
		}
	}
	return false;
}





