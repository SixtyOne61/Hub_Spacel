// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class SimplyHttpRequest
{
public:
    static void processRequest(class FHttpModule* _module, FString && _url, FString && _verb);
};
