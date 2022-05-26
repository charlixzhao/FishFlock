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
	TObjectPtr<ACharacter> Predator;
	FVector Centroid;
	EPredatorState PredatorState;
	float CentroidToPredatorDistance;
	float RippleForce;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config", meta=(AllowPrivateAccess="true"))
	float Fear;
	//float NearestNeighbourDistance;
	TMap<TObjectPtr<AFish>, TArray<TObjectPtr<AFish>>> NearestNeighbours;

	void EnterFastAvoid();
	
	void LeaveFastAvoid();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	TSubclassOf<AFish> FishClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	int32 FishNum;

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

	double max_speed_current;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	void InitFishPositions();
	void UpdateFishVelocities(float DeltaTime);
	void UpdateControlParameters(float DeltaTime);
	FVector Rule_1_Cohesion(AFish const* Fish);
	FVector Rule_2_Separation (AFish const* Fish);
	FVector Rule_3_Alignment(AFish const* Fish);
	TArray<TObjectPtr<AFish>> GetNearestNeighboursByPercentage(AFish const* Fish, float Percentage);

	void UpdateFishVelocities_Wander(float DeltaTime);
	//Herd
	void UpdateFishVelocities_Herd(float DeltaTime);
	
	//Ball
	void UpdateFishVelocities_Ball(float DeltaTime);
	//Ball
	FVector Ball_Get_Center(AFish const* Fish);
	FVector Ball_Rotate_Around(AFish const* Fish, const FVector& center);

	//Predator_attack
	void UpdateFishVelocities_FlashOutward(float DeltaTime);
	void UpdateFishVelocities_FlashInward(float DeltaTime);
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> ControllerMesh;

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TArray<TObjectPtr<AFish>> Fishes;
};


