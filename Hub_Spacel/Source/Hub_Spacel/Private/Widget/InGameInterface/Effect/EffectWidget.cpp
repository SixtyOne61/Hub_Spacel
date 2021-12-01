// Fill out your copyright notice in the Description page of Project Settings.


#include "EffectWidget.h"
#include "DataAsset/EffectDataAsset.h"

void UEffectWidget::SetEffect(EEffect _effect)
{
    if(this->EffectDataAsset == nullptr) return;

    if (auto const& effectParam = this->EffectDataAsset->getEffect(_effect))
    {
        this->Effect = effectParam->Type;
        this->Location = effectParam->TypeLocation;

        BP_Setup(effectParam->Icone, effectParam->BackgroundColor, effectParam->Title, effectParam->TypeLocation);
    }
}