// Fill out your copyright notice in the Description page of Project Settings.


#include "SpinCarrouselWidget.h"
#include "Components/GridPanel.h"
#include "Components/PanelSlot.h"
#include "Components/UniformGridSlot.h"
#include "Components/GridSlot.h"
#include "Util/SimplyUI.h"

void USpinCarrouselWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;

    TArray<FName> itemNames { TEXT("WBP_ItemCarrousel_Left"), TEXT("WBP_ItemCarrousel_Middle"), TEXT("WBP_ItemCarrousel_Right") };
    SimplyUI::initArray(this, Items, itemNames);

    GridCarrousel = SimplyUI::initSafetyFromName<UUserWidget, UGridPanel>(this, TEXT("GridPanel_Carrousel"));
}

void USpinCarrouselWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void USpinCarrouselWidget::setDesc()
{
    BP_SetTitle(this->Items[1]->m_data.m_title);
}

void USpinCarrouselWidget::setupItems(TArray<UItemCarrouselWidget::FData> const& _data)
{
    for (int i = 0; i < _data.Num(); ++i)
    {
        if (i < this->Items.Num())
        {
            this->Items[i]->setupItems(_data[i]);
        }
    }

    setDesc();
}

void USpinCarrouselWidget::SpinRight()
{
    UItemCarrouselWidget::FData save = this->Items.Last()->m_data;

    int32 max = this->Items.Num() - 1;
    for (int i = max; i >= 0; --i)
    {
        if (i - 1 >= 0)
        {
            UItemCarrouselWidget::FData swap = this->Items[i - 1]->m_data;
            this->Items[i]->setupItems(swap);
        }
        else
        {
            this->Items[i]->setupItems(save);
        }
    }

    setDesc();

    OnCarrouselMoveDelegate.Broadcast();
}

void USpinCarrouselWidget::SpinLeft()
{
    UItemCarrouselWidget::FData save = this->Items[0]->m_data;

    int32 max = this->Items.Num();
    for (int i = 0; i < max; ++i)
    {
        if (i + 1 < max)
        {
            UItemCarrouselWidget::FData swap = this->Items[i + 1]->m_data;
            this->Items[i]->setupItems(swap);
        }
        else
        {
            this->Items[i]->setupItems(save);
        }
    }

    setDesc();

    OnCarrouselMoveDelegate.Broadcast();
}

uint8 USpinCarrouselWidget::getIdSelected() const
{
    if (this->Items.Num() != 0 && this->Items[1] != nullptr)
    {
        return this->Items[1]->m_data.m_id;
    }

    ensure(false);
    return 0;
}