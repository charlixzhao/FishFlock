// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FishGroupAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class FISHFLOCK_API UFishGroupAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	FName GetCurrentStateName() const;
private:
	void UpdateControllerState(float DeltaTime);
	
};
