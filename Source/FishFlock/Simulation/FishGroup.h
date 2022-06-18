// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Fish.h"
#include "GameFramework/Actor.h"
#include "FishFlockTypes.h"
#include "FishGroup.generated.h"


class AFish;


UCLASS()
class FISHFLOCK_API AFishGroup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFishGroup();

	bool DoesAnyFishVision() const;
	bool DoesAnyFishFeelRipple() const;
	
	TObjectPtr<ACharacter> Predator;
	FVector Centroid;
	EPredatorState PredatorState;
	float CentroidToPredatorDistance;
	float NearestToPredatorDistance;
	float RippleForce;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config", meta=(AllowPrivateAccess="true"))
	float Fear;
	
	//float NearestNeighbourDistance;
	TMap<TObjectPtr<AFish>, TArray<TObjectPtr<AFish>>> NearestNeighbours;

	void EnterFastAvoid();
	void LeaveFastAvoid();

	void EnterSkitter();

	
	bool IsAnyFishFront();
	bool HasFlockShiftNinetyDegree();

	UPROPERTY(EditAnywhere, Category = "Flash")
	bool Flash_Initialized = false;

	UPROPERTY(EditAnywhere, Category = "Flash")
	FVector Flash_Return_Position = FVector(0,0,0);
	

	UPROPERTY(EditAnywhere, Category = "Hourglass")
	bool Hourglass_Initialized = false;

	UPROPERTY(EditAnywhere, Category = "Hourglass")
	FVector Hourglass_Initial_Vector;
	UPROPERTY(EditAnywhere, Category = "Hourglass")
	FVector Hourglass_Current_Vector;
	
	UPROPERTY(EditAnywhere, Category = "Hourglass")
	int Hourglass_Turning_Direction = 0; // 0 -> uninitialized, 1 -> left, 2 -> right

		
	UPROPERTY(EditAnywhere, Category = "Split")
	AFish* LeftLeader = nullptr;
	UPROPERTY(EditAnywhere, Category = "Split")
	AFish* RightLeader = nullptr;
	UPROPERTY(EditAnywhere, Category = "Split")
	bool Split_Initialized = false;
	
	UPROPERTY(EditAnywhere, Category = "Split")
	FVector LeftLeaderEscapeDirection = FVector(0,0,0);

	UPROPERTY(EditAnywhere, Category = "Split")
	FVector RightLeaderEscapeDirection = FVector(0,0,0);
	
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TArray<TObjectPtr<AFish>> SplitLeft;

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TArray<TObjectPtr<AFish>> SplitRight;

	UPROPERTY(EditAnywhere, Category = "Join")
	float AverageDistanceFromCentroid = 0.f;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	TSubclassOf<AFish> FishClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	int32 FishNum = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	int32 LeaderNum = 3;
	
	UPROPERTY(EditDefaultsOnly, Category = "Communication")
	float CommunicationInterval = 0.5f;
	
	//These defines how strong is each rule. Needs fine-tuning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid")
	double rule_1_scale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid")
	double rule_2_scale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid")
	double rule_3_scale;
	
	//These defines how close the boids should be to be considered in the rules
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid")
	double rule_1_dist;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid")
	double rule_2_dist;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid")
	double rule_3_dist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid")
	double max_speed_normal = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid")
	double max_speed_fast_avoid = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid")
	double max_acceleration = 1024.f;
	
	double max_speed_current;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid")
	double max_speed_escape = 300.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	void InitFishPositions();
	void RandomChooseLeaders();
	void UpdateFishVelocities(float DeltaTime);
	void UpdateControlParameters(float DeltaTime);
	FVector Rule_FollowLeaders(AFish const* Fish);
	FVector Rule_1_Cohesion(AFish const* Fish);
	FVector Rule_2_Separation (AFish const* Fish);
	FVector Rule_3_Alignment(AFish const* Fish);
	TArray<TObjectPtr<AFish>> GetNearestNeighboursByPercentage(AFish const* Fish, float Percentage);

	TArray<FVector> DetectNoCollisionDirections(AFish* Fish) const;
	
	void UpdateFishVelocities_Wander(float DeltaTime);
	void UpdateFishVelocities_Herd(float DeltaTime);
	void UpdateFishVelocities_Avoid(float DeltaTime);

	void UpdateFishVelocities_FastAvoid(float DeltaTime);

	void UpdateFishVelocities_Skitter(float DeltaTime);

	
	//Compact
	void UpdateFishVelocities_Compact(float DeltaTime);
	
	FVector Rule_2_Compact(AFish const* Fish);

	//Ball
	void UpdateFishVelocities_Ball(float DeltaTime);
	//Ball
	FVector Ball_Get_Center(AFish const* Fish);
	FVector Ball_Rotate_Around(AFish const* Fish, const FVector& center);
	
	//Predator_attack
	void UpdateFishVelocities_FlashOutward(float DeltaTime);
	void UpdateFishVelocities_FlashInward(float DeltaTime);

	
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawCollisionDebug;

	void UpdateFishVelocities_Split(float DeltaTime);
	void Split_and_FindLeader();

	void UpdateFishVelocities_Fountain(float DeltaTime);

	void UpdateFishVelocities_Hourglass(float DeltaTime);

	void UpdateFishVelocities_Join(float DeltaTime);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> SceneRoot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> ControllerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class USplineComponent> TravelSpline;
	
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TArray<TObjectPtr<AFish>> Fishes;



	
	
	TArray<AFish*> Leaders;

	friend class AFish;
	
};


