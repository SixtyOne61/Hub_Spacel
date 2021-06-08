// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerNameActor.h"
#include "Widget/PlayerNameWidget.h"
#include "Components/WidgetComponent.h"

void APlayerNameActor::setPlayerName(FString const& _name, FSlateColor const& _teamColor)
{
	if (!ensure(this->BillboardWidgetComponent != nullptr)) return;
	if (UPlayerNameWidget* playerNameWidget = Cast<UPlayerNameWidget>(this->BillboardWidgetComponent->GetUserWidgetObject()))
	{
		playerNameWidget->SetPlayerName(_name, _teamColor);
	}
}

void APlayerNameActor::show(bool _show)
{
	if (!ensure(this->BillboardWidgetComponent != nullptr)) return;
	if (UPlayerNameWidget* playerNameWidget = Cast<UPlayerNameWidget>(this->BillboardWidgetComponent->GetUserWidgetObject()))
	{
		playerNameWidget->SetVisibility(_show ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}