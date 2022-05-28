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

	UFUNCTION(BlueprintCallable)
	void EnterFastAvoid();

	
	UFUNCTION(BlueprintCallable)
	void LeaveFastAvoid();

protected:
	UPROPERTY(EditDefaultsOnly, Category="Config")
	FName StateMachineName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Threshold")
	float HerdDistance_Threshold = 90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Threshold")
	float Force_Threshold = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Threshold")
	float Fear_Threshold = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Threshold")
	float BallDistance_Threshold = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Threshold")
	float AvoidDistance_Threshold = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Threshold")
	float ChaseTime_Threshold = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Threshold")
	float HerdTime_Threshold = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Threshold")
	float SkitterTime_Threshold = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Threshold")
	float FlashTime_Threshold = 1.f;
protected:
	UPROPERTY(BlueprintReadOnly, Category="Transition")
	float CentroidToPredatorDistance;

	UPROPERTY(BlueprintReadOnly, Category="Transition")
	EPredatorState PredatorState;

	UPROPERTY(BlueprintReadOnly, Category="Transition")
	bool bVision;
		
	UPROPERTY(BlueprintReadOnly, Category="Transition")
	float RippleForce;
	
	UPROPERTY(BlueprintReadOnly, Category="Transition")
	float Fear;
	
	UPROPERTY(BlueprintReadWrite, Category="Transition")
	float SkitterTime = -1.f;
	
	UPROPERTY(BlueprintReadOnly, Category="Transition")
	float ChaseTime = -1.f;

	UPROPERTY(BlueprintReadWrite, Category="Transition")
	float HerdTime = -1.f;
	
	UPROPERTY(BlueprintReadWrite, Category="Transition")
	float FlashOutTime = -1.f;

	UPROPERTY(BlueprintReadWrite, Category="Transition")
	float FlashInTime = -1.f;

	UPROPERTY(BlueprintReadWrite, Category="Transition")
	float FountainTime = -1.f;

	UPROPERTY(BlueprintReadWrite, Category="Transition")
	float HourglassTime = -1.f;
private:
	void UpdatePredatorState();
private:
	TObjectPtr<class AFishGroup> FishGroup;
	
};
