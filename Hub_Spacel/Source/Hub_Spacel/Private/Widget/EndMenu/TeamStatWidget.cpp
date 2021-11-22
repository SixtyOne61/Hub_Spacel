// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamStatWidget.h"

FColorsType UTeamStatWidget::getColorType(FString const& _team) const
{
    if (this->Colors != nullptr)
    {
        return this->Colors->GetColorType(_team);
    }

    return FColorsType();
}

UTexture2D* UTeamStatWidget::GetLogo(FString const& _team) const
{
    auto data = getColorType(_team);
    return data.Logo;
}

FColor UTeamStatWidget::GetColor(FString const& _team) const
{
    auto data = getColorType(_team);
    return data.get<FColor>();
}