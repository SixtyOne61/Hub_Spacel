// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Server/ServerFinder.h"
#include "Server/ServerDesc.h"
#include "Hub_SpacelGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UHub_SpacelGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    UHub_SpacelGameInstance(FObjectInitializer const& _objectInitialize);

    UFUNCTION(BlueprintCallable)
    TArray<FServerDesc> const& GetServers() const;

    UFUNCTION(BlueprintCallable)
    void CleanServers();

    UFUNCTION(BlueprintCallable)
    void JoinServer(FText _ip) const;

    UFUNCTION(BlueprintCallable)
    void LoadMenu();

    /* reset input mode for first player controller */
    UFUNCTION(BlueprintCallable)
    void ResetInputMode() const;

    /* override init */
    virtual void Init() override;

private:
    TSubclassOf<class UUserWidget> m_mainMenuClass = nullptr;

    // server finder handle, use for find available server
    ServerFinder ServerFinderHandle = ServerFinder();
};