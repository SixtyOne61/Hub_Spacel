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

    if(this->ActionNames.Num() != this->Values.Num()) return;

    if (this->ActionNames.Num() == 0)
    {
        this->UpdateInputDelegate.Broadcast(this->Type, this->Key);
    }

    for (int i = 0; i < this->ActionNames.Num(); ++i)
    {
        FString const& name = this->ActionNames[i];
        if (name != FString{})
        {
            if (this->Values[i] != 0.0f)
            {
                BP_UpdateBindingAction(name, this->Key);
            }
            else
            {
                BP_UpdateBindingAxis(name, this->Key, this->Values[i]);
            }
        }
        else
        {
            this->UpdateInputDelegate.Broadcast(this->Type, this->Key);
        }
    }
}