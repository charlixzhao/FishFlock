// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Fish.generated.h"

UENUM()
enum EFish_State
{
	Default_State UMETA(DisplayName = "Default_State"),
	Vacuole UMETA(DisplayName = "Vacuole"),
	Fountain UMETA(DisplayName = "Fountain"),
	Hourglass UMETA(DisplayName = "Hourglass"),
	Flash UMETA(DisplayName = "Flash"),
	Split UMETA(DisplayName = "Split"),
};

UENUM()
enum EFish_SubStage
{
	Default_SubStage UMETA(DisplayName = "Default_SubStage"),
	Alert UMETA(DisplayName = "Alert"),
	Reaction UMETA(DisplayName = "Reaction"),
};

UENUM()
enum EFish_SubState
{
	Default_SubState UMETA(DisplayName = "Default_SubState"),
	Communicated UMETA(DisplayName = "Communicated"),
	Repulsive UMETA(DisplayName = "Repulsive"),
	Regrouping UMETA(DisplayName = "Regrouping"),
	Turn_Around UMETA(DisplayName = "Turn_Around"),
};


UCLASS()
class FISHFLOCK_API AFish : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFish();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UArrowComponent> Arrow;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class USphereComponent> SphereCollider;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Locomotion", meta=(AllowPrivateAccess="true"))
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "EFish_State", meta=(AllowPrivateAccess="true"))
	TEnumAsByte<EFish_State> Status;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "EFish_State", meta=(AllowPrivateAccess="true"))
	TEnumAsByte<EFish_SubStage> SubStage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "EFish_State", meta=(AllowPrivateAccess="true"))
	TEnumAsByte<EFish_SubState> SubState;
	
	
	friend class AFishGroup;
};


