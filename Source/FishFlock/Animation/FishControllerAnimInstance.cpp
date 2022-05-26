// Fill out your copyright notice in the Description page of Project Settings.


#include "FishControllerAnimInstance.h"

#include "Animation/AnimNode_StateMachine.h"
#include "Simulation/PredatorCharacter.h"
#include "Simulation/FishGroup.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

void UFishControllerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	FishGroup = Cast<AFishGroup>(GetOwningActor());
}


FName UFishControllerAnimInstance::GetCurrentStateName() const
{
	return GetStateMachineInstance(GetStateMachineIndex(StateMachineName))->GetCurrentStateName();
}

void UFishControllerAnimInstance::UpdateControllerState(float DeltaTime)
{
	if(!FishGroup)
	{
		FishGroup = Cast<AFishGroup>(GetOwningActor());
	}
	
	if(FishGroup)
	{
		CentroidToPredatorDistance = FishGroup->CentroidToPredatorDistance;
		bVision = FishGroup->DoesAnyFishVision();
		Fear = FishGroup->Fear;
		RippleForce = FishGroup->RippleForce;
		if(SkitterTime >= 0.f) SkitterTime += DeltaTime;
		if(HerdTime >= 0.f) HerdTime += DeltaTime;
		if(ChaseTime >= 0.f) ChaseTime += DeltaTime;
		UpdatePredatorState();
	}
	
}

void UFishControllerAnimInstance::EnterFastAvoid()
{
	if(FishGroup) FishGroup->EnterFastAvoid();
}

void UFishControllerAnimInstance::LeaveFastAvoid()
{
	if(FishGroup) FishGroup->LeaveFastAvoid();
}

void UFishControllerAnimInstance::UpdatePredatorState()
{
	const APredatorCharacter* PredatorCharacter =
			Cast<APredatorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if(PredatorCharacter)
	{
		const EPredatorState LastState = PredatorState;
		PredatorState = EPredatorState::None;
		if(bVision) PredatorState = EPredatorState::Presence;
		if(bVision && PredatorCharacter->PredatorState == EPredatorState::Chase) PredatorState = EPredatorState::Chase;
		if(bVision && PredatorCharacter->PredatorState == EPredatorState::Attack) PredatorState = EPredatorState::Attack;
		if(LastState != EPredatorState::Chase && PredatorState == EPredatorState::Chase) ChaseTime = 0.f;
		if(LastState == EPredatorState::Chase && PredatorState != EPredatorState::Chase) ChaseTime = -1.f;
	}
}
