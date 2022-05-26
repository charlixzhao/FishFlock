// Fill out your copyright notice in the Description page of Project Settings.


#include "Fish.h"

#include "FishGroup.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AFish::AFish()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollider"));
	if(!RootComponent) RootComponent = SphereCollider;
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardDirection"));
	Arrow->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFish::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFish::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector const FishForward = UKismetMathLibrary::GetForwardVector(GetActorRotation());
	FVector const DirectionToPredator = (BelongingGroup->Predator->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	float const Angle = FMath::RadiansToDegrees(FMath::Acos(FishForward.Dot(DirectionToPredator)));
	bVision = Angle <= 150.f && FVector::Distance(GetActorLocation(), BelongingGroup->Predator->GetActorLocation()) < VisionDistance;
}

