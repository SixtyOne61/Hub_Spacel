// Fill out your copyright notice in the Description page of Project Settings.


#include "Hub_SpacelGameInstance.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"

UHub_SpacelGameInstance::UHub_SpacelGameInstance(FObjectInitializer const& _objectInitialize)
{
    ConstructorHelpers::FClassFinder<UUserWidget> mainMenuBpClass(TEXT("/Game/05_UI/WBP_MainMenu"));
    if (!ensure(mainMenuBpClass.Class != nullptr)) return;

    MainMenuClass = mainMenuBpClass.Class;
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

void UHub_SpacelGameInstance::JoinServer() const
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
    if (!ensure(this->MainMenuClass != nullptr)) return;
    UUserWidget* mainMenu = CreateWidget<UUserWidget>(this, this->MainMenuClass);
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
    UE_LOG(LogTemp, Warning, TEXT("Found class %s"), *MainMenuClass->GetName());
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
