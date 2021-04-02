// Fill out your copyright notice in the Description page of Project Settings.

#include "TargetActor.h"
#include "Components/WidgetComponent.h"
#include "Widget/TargetUserWidget.h"

// Called when the game starts or when spawned
void ATargetActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (this->BillboardWidgetComponent != nullptr)
	{
		if (UTargetUserWidget* targetWidget = Cast<UTargetUserWidget>(this->BillboardWidgetComponent->GetUserWidgetObject()))
		{
			targetWidget->Owner = this;
		}
	}
}

void ATargetActor::showTarget(bool _show)
{
	if (!ensure(this->BillboardWidgetComponent != nullptr)) return;
	if (UTargetUserWidget* targetWidget = Cast<UTargetUserWidget>(this->BillboardWidgetComponent->GetUserWidgetObject()))
	{
		targetWidget->showTarget(_show);
	}
}