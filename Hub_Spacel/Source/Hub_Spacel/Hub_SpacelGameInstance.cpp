// Fill out your copyright notice in the Description page of Project Settings.


#include "Hub_SpacelGameInstance.h"
#include "Engine/World.h"

void UHub_SpacelGameInstance::CreateServer() const
{
    UWorld* world = GetWorld();
    if (!ensure(world != nullptr)) return;

    UEngine* engine = GetEngine();
    if (!ensure(engine != nullptr)) return;

    engine->AddOnScreenDebugMessage(0, 5.0f, FColor::Green, TEXT("Create Server"));

    world->ServerTravel("/Game/03_Level/Standalone/InGameLevel?listen");
}

void UHub_SpacelGameInstance::JoinServer() const
{
    UEngine* engine = GetEngine();
    if (!ensure(engine != nullptr)) return;

    engine->AddOnScreenDebugMessage(0, 5.0f, FColor::Green, TEXT("Joining Server"));

    APlayerController* playerController = GetFirstLocalPlayerController();
    if (!ensure(playerController != nullptr)) return;

    playerController->ClientTravel("192.168.1.82", ETravelType::TRAVEL_Absolute);
}
