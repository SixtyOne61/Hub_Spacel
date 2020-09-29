// Fill out your copyright notice in the Description page of Project Settings.


#include "Hub_SpacelGameInstance.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "Source/Factory/SpacelFactory.h"

UHub_SpacelGameInstance::UHub_SpacelGameInstance(FObjectInitializer const& _objectInitialize)
{
    m_mainMenuClass = SpacelFactory::FindClass<UUserWidget>(TEXT("/Game/UI/MainMenu/WBP_MainMenu"));
}

TArray<FServerDesc> const& UHub_SpacelGameInstance::GetServers() const
{
    return this->ServerFinderHandle.GetServers();
}

void UHub_SpacelGameInstance::JoinServer(FText _ip) const
{
    UEngine* engine = this->GetEngine();
    if (!ensure(engine != nullptr)) return;

    engine->AddOnScreenDebugMessage(0, 5.0f, FColor::Green, TEXT("Joining Server"));

    UWorld* world = this->GetWorld();
    if (!ensure(world != nullptr)) return;

    FString cmd = "Open ";
    cmd.Append(_ip.ToString());
    world->Exec(world, *cmd);
}

void UHub_SpacelGameInstance::CreateServer() const
{
    UWorld* world = GetWorld();
    if (!ensure(world != nullptr)) return;

    UEngine* engine = GetEngine();
    if (!ensure(engine != nullptr)) return;

    engine->AddOnScreenDebugMessage(0, 5.0f, FColor::Green, TEXT("Create Server"));

    world->ServerTravel("/Game/03_Level/Standalone/InGameLevel?listen");
}

void UHub_SpacelGameInstance::JoinServerOld() const
{
    UEngine* engine = GetEngine();
    if (!ensure(engine != nullptr)) return;

    engine->AddOnScreenDebugMessage(0, 5.0f, FColor::Green, TEXT("Joining Server"));

    APlayerController* playerController = this->GetFirstLocalPlayerController();
    if (!ensure(playerController != nullptr)) return;

    playerController->ClientTravel("192.168.1.77", ETravelType::TRAVEL_Absolute);
}

void UHub_SpacelGameInstance::LoadMenu()
{
    if (!ensure(m_mainMenuClass != nullptr)) return;
    UUserWidget* mainMenu = CreateWidget<UUserWidget>(this, m_mainMenuClass);
    if (!ensure(mainMenu != nullptr)) return;

    mainMenu->bIsFocusable = true;
    mainMenu->AddToViewport();

    APlayerController* playerController = this->GetFirstLocalPlayerController();
    if (!ensure(playerController != nullptr)) return;

    FInputModeUIOnly inputMode;
    inputMode.SetWidgetToFocus(mainMenu->TakeWidget());
    inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

    // set input mode to player controller
    playerController->SetInputMode(inputMode);
    playerController->bShowMouseCursor = true;
}

void UHub_SpacelGameInstance::Init()
{
    UE_LOG(LogTemp, Warning, TEXT("Found class %s"), *this->m_mainMenuClass->GetName());
}

void UHub_SpacelGameInstance::ResetInputMode() const
{
    APlayerController* playerController = this->GetFirstLocalPlayerController();
    if (!ensure(playerController != nullptr)) return;

    FInputModeGameOnly inputMode;
    inputMode.SetConsumeCaptureMouseDown(false);

    playerController->SetInputMode(inputMode);
    playerController->bShowMouseCursor = false;
}
