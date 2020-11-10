// Fill out your copyright notice in the Description page of Project Settings.
#include "MainMenuHUD.h"
#include "Blueprint/UserWidget.h"

AMainMenuHUD::AMainMenuHUD()
{
    static ConstructorHelpers::FClassFinder<UUserWidget> mainMenuObj(TEXT("/Game/Blueprint/UI/Widgets/WBP_MainMenu"));
    MainMenuWidgetClass = mainMenuObj.Class;
}

void AMainMenuHUD::BeginPlay()
{
    Super::BeginPlay();

    UWorld* world = this->GetWorld();
    if (!ensure(world != nullptr)) return;

    APlayerController* playerController = world->GetFirstPlayerController();
    if (!ensure(playerController != nullptr)) return;

    // player need to see cursor on main menu for navigation
    playerController->bShowMouseCursor = true;

    // add user widget to viewport
    UUserWidget* mainMenuWidget = CreateWidget<UUserWidget>(world, this->MainMenuWidgetClass);
    if (!ensure(mainMenuWidget != nullptr)) return;
    mainMenuWidget->AddToViewport();
    mainMenuWidget->SetFocus();
}