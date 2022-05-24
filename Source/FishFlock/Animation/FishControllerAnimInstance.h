// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FishFlockTypes.h"
#include "FishControllerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class FISHFLOCK_API UFishControllerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	FName GetCurrentStateName() const;
	
	UFUNCTION(BlueprintCallable)
	void UpdateControllerState(float DeltaTime);


protected:
	UPROPERTY(EditDefaultsOnly, Category="Config")
	FName StateMachineName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
	float VisionDistance = 300.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Threshold")
	float DistanceToPredator_Threshold = 300.f;

	
protected:
	UPROPERTY(BlueprintReadOnly, Category="Predator")
	float CentroidToPredatorDistance;

	UPROPERTY(BlueprintReadOnly, Category="Predator")
	EPredatorState PredatorState;

	UPROPERTY(BlueprintReadOnly, Category="Vision")
	bool bVision;

	UPROPERTY(BlueprintReadOnly, Category="Communication")
	bool bCommunicationFactor;

	UPROPERTY(BlueprintReadOnly, Category="Communication")
	float AverageInformationTransfer;
	
	UPROPERTY(BlueprintReadOnly, Category="Lateral")
	float NearestNeighbourDistance;
		
	UPROPERTY(BlueprintReadOnly, Category="Lateral")
	float RippleForce;
	
	UPROPERTY(BlueprintReadOnly, Category="Transition")
	float Fear;
	
	UPROPERTY(BlueprintReadOnly, Category="Transition")
	float SkitterDistance;

private:

	TObjectPtr<class AFish> Fish;
	
};
