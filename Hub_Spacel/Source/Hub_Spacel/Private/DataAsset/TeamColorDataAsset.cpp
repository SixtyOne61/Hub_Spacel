// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamColorDataAsset.h"

FSlateColor UTeamColorDataAsset::GetColor(FString const& _team) const
{
    if (_team == "Team 1")
    {
        return this->Team1;
    }
    else if (_team == "Team 2")
    {
        return this->Team2;
    }
    else if (_team == "Team 3")
    {
        return this->Team3;
    }
    else if (_team == "Team 4")
    {
        return this->Team4;
    }
    else if (_team == "Team 5")
    {
        return this->Team5;
    }
    else if (_team == "Team 6")
    {
        return this->Team6;
    }

    ensure(false);
    return FSlateColor();
}