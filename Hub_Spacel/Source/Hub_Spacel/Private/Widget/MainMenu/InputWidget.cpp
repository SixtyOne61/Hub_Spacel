// Fill out your copyright notice in the Description page of Project Settings.


#include "InputWidget.h"
#include "DataAsset/KeyDataAsset.h"

void UInputWidget::OnNewKeySelected(FKey const& _key)
{
    this->Key = _key;

    if (this->KeyDataAsset != nullptr)
    {
        BP_UpdateKeyTexture(this->KeyDataAsset->get(_key));
    }

    if (this->ActionName != FString{})
    {
        if (this->Value != 0.0f)
        {
            BP_UpdateBindingAction(this->ActionName, this->Key);
        }
        else
        {
            BP_UpdateBindingAxis(this->ActionName, this->Key, this->Value);
        }
    }
    else
    {
        this->UpdateInputDelegate.Broadcast(this->Type, this->Key);
    }
}