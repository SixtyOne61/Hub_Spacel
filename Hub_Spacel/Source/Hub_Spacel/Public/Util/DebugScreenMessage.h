// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"

namespace DebugScreenMessage
{
    static void constexpr Message(float _duration, FString && _text, FColor _color = FColor::Green)
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, _duration, _color, _text);
    }
}