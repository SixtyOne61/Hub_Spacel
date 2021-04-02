// Fill out your copyright notice in the Description page of Project Settings.


#include "BillboardActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"

// Sets default values
ABillboardActor::ABillboardActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABillboardActor::BeginPlay()
{
	Super::BeginPlay();

	this->BillboardWidgetComponent = Cast<UWidgetComponent>(this->GetComponentByClass(UWidgetComponent::StaticClass()));
}

// Called every frame
void ABillboardActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* world = this->GetWorld();
	if (!ensure(world != nullptr)) return;

	APawn const* pawn = UGameplayStatics::GetPlayerPawn(world, 0);
	if (pawn == nullptr) return;

	FVector const& pawnLocation = pawn->GetActorLocation();

	if (!ensure(this->BillboardWidgetComponent != nullptr)) return;
	FVector const& widgetLocation = this->BillboardWidgetComponent->GetComponentLocation();

	FRotator rot = UKismetMathLibrary::FindLookAtRotation(widgetLocation, pawnLocation);

	this->BillboardWidgetComponent->SetWorldRotation(rot);
}

