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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
	TSubclassOf<class AFish> FishClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
	int32 FishNum;

	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	void InitFishPositions();
	void UpdateFishVelocities(float DeltaTime);
	
private:
	TArray<class AFish*> Fishes;

};


