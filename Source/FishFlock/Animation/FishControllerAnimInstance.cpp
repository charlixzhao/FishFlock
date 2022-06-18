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

FString UFishControllerAnimInstance::BP_GetCurrentStateName() const
{
	return GetCurrentStateName().ToString();
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
		bFeelRipple = FishGroup->DoesAnyFishFeelRipple();
		Fear = FishGroup->Fear;
		RippleForce = FishGroup->RippleForce;
		if(FastAvoidTime >= 0.f) FastAvoidTime += DeltaTime;
		if(SkitterTime >= 0.f) SkitterTime += DeltaTime;
		if(HerdTime >= 0.f) HerdTime += DeltaTime;
		if(ChaseTime >= 0.f) ChaseTime += DeltaTime;
		if(FlashInTime >= 0.f) FlashInTime += DeltaTime;
		if(FlashOutTime >= 0.f) FlashOutTime += DeltaTime;
		if(FountainTime >= 0.f)
		{
			FountainTime += DeltaTime;
			//TODO add frontal fish check if Fountain behaves correctly
			//FishGroup->IsAnyFishFront();
		}
		if(HourglassTime >= 0.f)
		{
			HourglassTime += DeltaTime;
			//TODO add 90 Degree check if Hourglass behaves correctly
			//FishGroup->HasFlockShiftNinetyDegree();
		}
		UpdatePredatorState();
	}
	
}

void UFishControllerAnimInstance::EnterFastAvoid()
{
	FastAvoidTime = 0.f;
	if(FishGroup) FishGroup->EnterFastAvoid();
}

void UFishControllerAnimInstance::LeaveFastAvoid()
{
	FastAvoidTime = -1.f;
	if(FishGroup) FishGroup->LeaveFastAvoid();
}

void UFishControllerAnimInstance::EnterSkitter()
{
	SkitterTime = 0.f;
	if(FishGroup)
	{
		FishGroup->EnterSkitter();
	}
}

void UFishControllerAnimInstance::LeaveSkitter()
{
	SkitterTime = -1.f;
}

void UFishControllerAnimInstance::ResetHourglassDirection()
{
	if(FishGroup)
	{
		FishGroup->Hourglass_Turning_Direction = 0;
		FishGroup->Hourglass_Initialized = false;
		FishGroup->Hourglass_Initial_Vector = FVector(0,0,0);
		FishGroup->Hourglass_Current_Vector = FVector(0,0,0);
	}
}

void UFishControllerAnimInstance::UpdatePredatorState()
{
	const APredatorCharacter* PredatorCharacter =
			Cast<APredatorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if(PredatorCharacter)
	{
		const EPredatorState LastState = PredatorState;
		PredatorState = EPredatorState::None;
		const bool bPredatorNear = bVision || bFeelRipple;
		if(bPredatorNear) PredatorState = EPredatorState::Presence;
		if(bPredatorNear && PredatorCharacter->PredatorState == EPredatorState::Chase) PredatorState = EPredatorState::Chase;
		if(bPredatorNear && PredatorCharacter->PredatorState == EPredatorState::Attack) PredatorState = EPredatorState::Attack;
		if(LastState != EPredatorState::Chase && PredatorState == EPredatorState::Chase) ChaseTime = 0.f;
		if(LastState == EPredatorState::Chase && PredatorState != EPredatorState::Chase) ChaseTime = -1.f;
	}
}
