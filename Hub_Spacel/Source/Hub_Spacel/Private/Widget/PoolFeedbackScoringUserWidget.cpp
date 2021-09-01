// Fill out your copyright notice in the Description page of Project Settings.


#include "PoolFeedbackScoringUserWidget.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Util/SimplyUI.h"
#include "Player/ShipPawn.h"
#include "Widget/PointUserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UPoolFeedbackScoringUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    TArray<FName> pointNames;
    for (int i = 0; i < 20; ++i)
    {
        FString str = "Point" + FString::FromInt(i);
        pointNames.Add(*str);
    }

    SimplyUI::initArray(this, AvailableWidget, pointNames);

    for (UPointUserWidget* pointUserWidget : AvailableWidget)
    {
        if (pointUserWidget != nullptr)
        {
            pointUserWidget->OnAnimEndDelegate.AddDynamic(this, &UPoolFeedbackScoringUserWidget::OnAnimationEnd);
        }
    }

    if (AShipPawn * shipPawn = this->GetOwningPlayerPawn<AShipPawn>())
    {
        shipPawn->OnFeedbackScoreDelegate.AddDynamic(this, &UPoolFeedbackScoringUserWidget::OnScored);
    }
}

void UPoolFeedbackScoringUserWidget::OnScored(EScoreType _type, int32 _value)
{
    if (this->AvailableWidget.Num())
    {
        if (UPointUserWidget* widget = this->AvailableWidget.Pop())
        {
            FString value = "+";
            value += FString::FromInt(_value);

            FVector2D mousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(this->GetWorld());
            if (UCanvasPanelSlot* panelSlot = Cast<UCanvasPanelSlot>(widget->Slot))
            {
                panelSlot->SetPosition(mousePosition);
                widget->StartAnim(value);

                BP_OnScored(_type);
            }
        }
    }
}

void UPoolFeedbackScoringUserWidget::OnAnimationEnd(UPointUserWidget* _widget)
{
    if (_widget)
    {
        AvailableWidget.Add(_widget);
    }
}