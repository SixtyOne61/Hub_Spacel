// Fill out your copyright notice in the Description page of Project Settings.

#include "TargetActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"
#include "Widget/TargetUserWidget.h"

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
	
	this->TargetWidgetComponent = Cast<UWidgetComponent>(this->GetComponentByClass(UWidgetComponent::StaticClass()));
	if (this->TargetWidgetComponent != nullptr)
	{
		if(UTargetUserWidget* targetWidget = Cast<UTargetUserWidget>(this->TargetWidgetComponent->GetUserWidgetObject()))
		{
			targetWidget->Owner = this;
		}
	}
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

	if (!ensure(this->TargetWidgetComponent != nullptr)) return;
	FVector const& widgetLocation { this->TargetWidgetComponent->GetComponentLocation() };

	FRotator rot { UKismetMathLibrary::FindLookAtRotation(widgetLocation, pawnLocation) };
	rot.Roll = 90.0f;

	if (!ensure(this->TargetWidgetComponent != nullptr)) return;
	this->TargetWidgetComponent->SetWorldRotation(rot);
}

void ATargetActor::showTarget(bool _show)
{
	this->TargetWidgetComponent->SetVisibility(_show, true);
}