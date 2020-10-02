// Fill out your copyright notice in the Description page of Project Settings.


#include "Hub_SpacelGameInstance.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "Source/Factory/SpacelFactory.h"
#include "Kismet/KismetSystemLibrary.h"

UHub_SpacelGameInstance::UHub_SpacelGameInstance(FObjectInitializer const& _objectInitialize)
{
    m_mainMenuClass = SpacelFactory::FindClass<UUserWidget>(TEXT("/Game/UI/MainMenu/WBP_MainMenu"));
}

TArray<FServerDesc> const& UHub_SpacelGameInstance::GetServers() const
{
    return this->ServerFinderHandle.GetServers();
}

void UHub_SpacelGameInstance::CleanServers()
{
    this->ServerFinderHandle.CleanServers();
}

void UHub_SpacelGameInstance::JoinServer(FText _ip) const
{
    UEngine* engine = this->GetEngine();
    if (!ensure(engine != nullptr)) return;

    engine->AddOnScreenDebugMessage(0, 5.0f, FColor::Green, TEXT("Joining Server"));

    UWorld* world = this->GetWorld();
    if (!ensure(world != nullptr)) return;

    FString cmd = "open ";
    cmd.Append(_ip.ToString());
    UKismetSystemLibrary::ExecuteConsoleCommand(world, cmd, this->GetFirstLocalPlayerController());
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
    UWorld* world = this->GetWorld();
    if (!ensure(world != nullptr)) return;

    FString cmd = "NetworkVersionOverride 12345";
    world->Exec(world, *cmd);
}

void UHub_SpacelGameInstance::ResetInputMode() const
{
    APlayerController* playerController = this->GetFirstLocalPlayerController();
    if (playerController == nullptr)
    {
        return;
    }

    FInputModeGameOnly inputMode;
    inputMode.SetConsumeCaptureMouseDown(false);

    playerController->SetInputMode(inputMode);
    playerController->bShowMouseCursor = false;
}
