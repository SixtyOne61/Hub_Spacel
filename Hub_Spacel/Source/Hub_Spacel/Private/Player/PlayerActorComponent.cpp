// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerActorComponent.h"
#include "Player/Common/CommonPawn.h"

// Called when the game starts
void UPlayerActorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	initShipPawnOwner();
}

bool UPlayerActorComponent::initShipPawnOwner()
{
	m_shipPawnOwner = MakeWeakObjectPtr(Cast<ACommonPawn>(this->GetOwner()));

	return m_shipPawnOwner.IsValid();
}