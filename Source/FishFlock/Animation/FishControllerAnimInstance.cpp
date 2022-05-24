// Fill out your copyright notice in the Description page of Project Settings.


#include "FishControllerAnimInstance.h"

#include "Animation/AnimNode_StateMachine.h"
#include "FishFlock/Simulation/Fish.h"
#include "GameFramework/Character.h"
#include "Simulation/FishGroup.h"
#include "Kismet/KismetMathLibrary.h"

void UFishControllerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Fish = Cast<AFish>(GetOwningActor());
}


FName UFishControllerAnimInstance::GetCurrentStateName() const
{
	return GetStateMachineInstance(GetStateMachineIndex(StateMachineName))->GetCurrentStateName();
}

void UFishControllerAnimInstance::UpdateControllerState(float DeltaTime)
{
	if(!Fish)
	{
		Fish = Cast<AFish>(GetOwningActor());
	}
	
	if(Fish && Fish->BelongingGroup)
	{
		CentroidToPredatorDistance = Fish->BelongingGroup->CentroidToPredatorDistance;
		bVision = Fish->bVision;
		bCommunicationFactor = Fish->bCommunicationFactor;
		AverageInformationTransfer = Fish->BelongingGroup->AverageInformationTransfer;
		NearestNeighbourDistance = Fish->BelongingGroup->NearestNeighbourDistance;
		Fear = Fish->RippleForce;

		//TODO: update ripple force, predator state, SkitterDistance
	}
	
}
