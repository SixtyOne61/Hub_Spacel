// Fill out your copyright notice in the Description page of Project Settings.
#include "MainMenuHUD.h"
#include "Blueprint/UserWidget.h"
#include "Factory/SpacelFactory.h"

AMainMenuHUD::AMainMenuHUD()
{
    MainMenuWidgetClass = SpacelFactory::FindClass<UUserWidget>(TEXT("/Game/Blueprint/UI/Widgets/WBP_MainMenu"));
}

void AMainMenuHUD::BeginPlay()
{
    Super::BeginPlay();

    UWorld* world { this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    APlayerController* playerController { world->GetFirstPlayerController() };
    if (!ensure(playerController != nullptr)) return;

    // player need to see cursor on main menu for navigation
    playerController->bShowMouseCursor = true;

    // add user widget to viewport
    SpacelFactory::createWidget<UUserWidget>(world, this->MainMenuWidgetClass, true);
}