// Fill out your copyright notice in the Description page of Project Settings.


#include "TutoPawn.h"
#include "Player/LocalPlayerActionComponent.h"
#include "Player/FireComponent.h"
#include "Player/ModuleComponent.h"
#include "Player/CustomCollisionComponent.h"
#include "Player/RepairComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/SkillComponent.h"
#include "DataAsset/TeamColorDataAsset.h"

// Sets default values
ATutoPawn::ATutoPawn()
	: Super()
{

}

// Called when the game starts or when spawned
void ATutoPawn::BeginPlay()
{
	Super::BeginPlay();

	activateComponent(this->FireComponent);
	activateComponent(this->RepairComponent);
	activateComponent(this->SkillComponent);

    this->WidgetTargetComponent = Cast<UWidgetInteractionComponent>(this->GetComponentByClass(UWidgetInteractionComponent::StaticClass()));

    // add speed line component
    if (ULocalPlayerActionComponent* localPlayerActionComponent = NewObject<ULocalPlayerActionComponent>(this, "LocalPlayerAction_00"))
    {
        localPlayerActionComponent->RegisterComponent();
    }

    // remove collision for local player (for disable hit with cursor for target)
    setCollisionProfile("NoOverlapTeam");
    this->DriverMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    activateComponent(this->SkillComponent);

    // add custom collision component
    if (UCustomCollisionComponent* customCollisionComponent = NewObject<UCustomCollisionComponent>(this, "CustomCollision_00"))
    {
        customCollisionComponent->RegisterComponent();
    }

    if (this->ShieldComponent != nullptr)
    {
        if (this->TeamColorDataAsset != nullptr)
        {
            FColor color = this->TeamColorDataAsset->GetColor<FColor>("Team 1");
            this->ShieldComponent->SetVectorParameterValueOnMaterials("Color", FVector{ color.ReinterpretAsLinear() });
        }
    }

    //this->ModuleComponent->OnStartGame();
}

// Called every frame
void ATutoPawn::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

	moveShip(_deltaTime);
}
