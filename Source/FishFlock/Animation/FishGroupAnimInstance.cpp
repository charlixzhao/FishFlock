// Fill out your copyright notice in the Description page of Project Settings.


#include "FishGroupAnimInstance.h"

#include "Animation/AnimNode_StateMachine.h"

void UFishGroupAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	UpdateControllerState(DeltaSeconds);
}

FName UFishGroupAnimInstance::GetCurrentStateName() const
{
	return GetStateMachineInstance(GetStateMachineIndex(FName("Control")))->GetCurrentStateName();
}

void UFishGroupAnimInstance::UpdateControllerState(float DeltaTime)
{
	//TODO: write the updating of transition parameters
	
}
