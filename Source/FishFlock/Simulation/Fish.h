// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Fish.generated.h"

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
	TObjectPtr<USkeletalMeshComponent> ControllerMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UArrowComponent> Arrow;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class USphereComponent> SphereCollider;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Locomotion", meta=(AllowPrivateAccess="true"))
	FVector Velocity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config", meta=(AllowPrivateAccess="true"))
	float Fear;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config", meta=(AllowPrivateAccess="true"))
	float VisionDistance = 300.f;
	

private:
	int32 Index = -1;
	TObjectPtr<class AFishGroup> BelongingGroup;
	
	
	bool bVision;
	bool bCommunicationFactor;
	float RippleForce;
	float SkitterDistance;
	
	friend struct FFishCommunicationSystem;
	friend class AFishGroup;
	friend class UFishControllerAnimInstance;
};


