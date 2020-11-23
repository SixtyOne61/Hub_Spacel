// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelHUD.h"
#include "Factory/SpacelFactory.h"
#include "Blueprint/UserWidget.h"

ASpacelHUD::ASpacelHUD()
{
    GameWidgetClass = SpacelFactory::FindClass<UUserWidget>(TEXT("/Game/Blueprint/UI/Widgets/WBP_Game"));
}

void ASpacelHUD::BeginPlay()
{
    Super::BeginPlay();

    UWorld* world = this->GetWorld();
    if (!ensure(world != nullptr)) return;

    APlayerController* playerController = world->GetFirstPlayerController();
    if (!ensure(playerController != nullptr)) return;

    playerController->bShowMouseCursor = false;

    // add user widget to viewport
    SpacelFactory::createWidget<UUserWidget>(world, this->GameWidgetClass, false);
}