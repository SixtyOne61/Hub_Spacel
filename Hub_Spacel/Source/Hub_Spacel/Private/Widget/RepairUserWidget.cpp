// Fill out your copyright notice in the Description page of Project Settings.


#include "RepairUserWidget.h"
#include "Util/SimplyUI.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Player/PlayerShipController.h"

void URepairUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (APlayerShipController* playerShipController = Cast<APlayerShipController>(UGameplayStatics::GetPlayerController(this->GetWorld(), 0)))
    {
        ButtonLinks.Add({ SimplyUI::initSafetyFromName<UUserWidget, UButton>(this, TEXT("Btn_Protection")), false, EKeys::Gamepad_DPad_Up, std::bind(&APlayerShipController::ToggleRepairProtection, playerShipController, std::placeholders::_1) });
        ButtonLinks.Add({ SimplyUI::initSafetyFromName<UUserWidget, UButton>(this, TEXT("Btn_Support")), false, EKeys::Gamepad_DPad_Right, std::bind(&APlayerShipController::ToggleRepairSupport, playerShipController, std::placeholders::_1) });
        ButtonLinks.Add({ SimplyUI::initSafetyFromName<UUserWidget, UButton>(this, TEXT("Btn_Ally1")), false, EKeys::Gamepad_DPad_Down, std::bind(&APlayerShipController::ToggleGiveAlly1, playerShipController, std::placeholders::_1) });
        ButtonLinks.Add({ SimplyUI::initSafetyFromName<UUserWidget, UButton>(this, TEXT("Btn_Ally2")), false, EKeys::Gamepad_DPad_Left, std::bind(&APlayerShipController::ToggleGiveAlly2, playerShipController, std::placeholders::_1) });
    }

    BaseBackgroundColor = FLinearColor::White;

    for (SButtonLinkInformation& btnLink : ButtonLinks)
    {
        if (btnLink.Button != nullptr)
        {
            btnLink.Button->SetBackgroundColor(BaseBackgroundColor);
        }
    }
}

void URepairUserWidget::NativeTick(const FGeometry& _myGeometry, float _deltaTime)
{
    Super::NativeTick(_myGeometry, _deltaTime);
    APlayerController* playerController { UGameplayStatics::GetPlayerController(this->GetWorld(), 0) };
    if(playerController == nullptr) return;

    for (SButtonLinkInformation& btnLink : ButtonLinks)
    {
        float analogKey = playerController->GetInputAnalogKeyState(btnLink.InputKey);
        if (analogKey >= 1.0f && !btnLink.IsPressed)
        {
            btnLink.IsPressed = true;
            btnLink.Callback(btnLink.IsPressed);
            if (btnLink.Button != nullptr)
            {
                btnLink.Button->SetBackgroundColor(this->PressedBackgroundColor);
            }
        }
        else if (analogKey < 1.0f && btnLink.IsPressed)
        {
            btnLink.IsPressed = false;
            btnLink.Callback(btnLink.IsPressed);
            if (btnLink.Button != nullptr)
            {
                btnLink.Button->SetBackgroundColor(this->BaseBackgroundColor);
            }
        }
    }
}

void URepairUserWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

