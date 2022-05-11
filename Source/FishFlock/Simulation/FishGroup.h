// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishGroup.generated.h"

UCLASS()
class FISHFLOCK_API AFishGroup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFishGroup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	TSubclassOf<class AFish> FishClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	int32 FishNum;

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
	
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	void InitFishPositions();
	void UpdateFishVelocities(float DeltaTime);
	FVector Rule_1_Cohesion(AFish* Fish);
	FVector Rule_2_Seperation(AFish* Fish);
	FVector Rule_3_Alignment(AFish* Fish);


private:
	TArray<class AFish*> Fishes;
};


