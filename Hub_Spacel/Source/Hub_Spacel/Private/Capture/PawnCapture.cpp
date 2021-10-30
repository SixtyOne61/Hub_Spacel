// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnCapture.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Mesh/SpacelInstancedMeshComponent.h"
#include "Util/SimplyXml.h"
#include "DataAsset/SetupAttributeDataAsset.h"

// Sets default values
APawnCapture::APawnCapture()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    DriverMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Driver_00"));
    if (!ensure(DriverMeshComponent != nullptr)) return;
    RootComponent = DriverMeshComponent;

    BaseShipMeshComponent = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("ShipBase_00"));
    if (!ensure(BaseShipMeshComponent != nullptr)) return;
    BaseShipMeshComponent->SetupAttachment(RootComponent);

    EmergencyComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Emergency_00"));
    if (!ensure(EmergencyComponent != nullptr)) return;
    EmergencyComponent->SetupAttachment(BaseShipMeshComponent);

    WeaponComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Weapon_00"));
    if (!ensure(WeaponComponent != nullptr)) return;
    WeaponComponent->SetupAttachment(BaseShipMeshComponent);

    ProtectionComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Protection_00"));
    if (!ensure(ProtectionComponent != nullptr)) return;
    ProtectionComponent->SetupAttachment(BaseShipMeshComponent);

    SupportComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Support_00"));
    if (!ensure(SupportComponent != nullptr)) return;
    SupportComponent->SetupAttachment(BaseShipMeshComponent);
}

void APawnCapture::BuildShip(EFormType _type)
{
    if (this->EmergencyComponent != nullptr)
    {
        this->EmergencyComponent->UseForm(_type, true);
    }

    if (this->WeaponComponent != nullptr)
    {
        this->WeaponComponent->UseForm(_type, true);
    }

    if (this->ProtectionComponent != nullptr)
    {
        this->ProtectionComponent->UseForm(_type, true);
    }

    if (this->SupportComponent != nullptr)
    {
        this->SupportComponent->UseForm(_type, true);
    }
}

// Called when the game starts or when spawned
void APawnCapture::BeginPlay()
{
	Super::BeginPlay();
    m_currentTime = this->Time;
}

// Called every frame
void APawnCapture::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

    m_currentTime -= _deltaTime;
    if (m_currentTime <= 0.0f)
    {
        m_currentForm = (EFormType)((uint8)m_currentForm + 1);
        if(m_currentForm == EFormType::Max) m_currentForm = EFormType::Base;

        BuildShip(m_currentForm);
        m_currentTime = this->Time;
    }
}

