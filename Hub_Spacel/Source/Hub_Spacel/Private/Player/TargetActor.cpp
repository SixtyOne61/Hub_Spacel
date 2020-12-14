// Fill out your copyright notice in the Description page of Project Settings.

#include "TargetActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"

// Sets default values
ATargetActor::ATargetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATargetActor::BeginPlay()
{
	Super::BeginPlay();
	
	this->TargetWidget = Cast<UWidgetComponent>(this->GetComponentByClass(UWidgetComponent::StaticClass()));
}

// Called every frame
void ATargetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* world { this->GetWorld() };
	if (!ensure(world != nullptr)) return;

	APawn* pawn { UGameplayStatics::GetPlayerPawn(world, 0) };
	if(pawn == nullptr) return;

	FVector const& pawnLocation { pawn->GetActorLocation() };

	if (!ensure(this->TargetWidget != nullptr)) return;
	FVector const& widgetLocation { this->TargetWidget->GetComponentLocation() };

	FRotator rot { UKismetMathLibrary::FindLookAtRotation(widgetLocation, pawnLocation) };
	rot.Roll = 90.0f;

	if (!ensure(this->TargetWidget != nullptr)) return;
	this->TargetWidget->SetWorldRotation(rot);
}

