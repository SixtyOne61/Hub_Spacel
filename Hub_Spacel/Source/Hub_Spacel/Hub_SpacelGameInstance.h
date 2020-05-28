// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Hub_SpacelGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UHub_SpacelGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ADefaultShell> ShellModuleClass = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ADefaultSubMachine> SubMachineModuleClass = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ADefaultEngine> EngineModuleClass = nullptr;
};