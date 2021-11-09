// Fill out your copyright notice in the Description page of Project Settings.


#include "Pirate.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DataAsset/PirateDataAsset.h"
#include "Util/SimplyXml.h"
#include "Util/Tag.h"
#include "Util/DebugScreenMessage.h"
#include "Gameplay/Bullet/ProjectileBase.h"
#include "Mesh/SpacelInstancedMeshComponent.h"

// Sets default values
APirate::APirate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RedCube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RedCube"));
	RootComponent = RedCube;

    WeaponComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Weapon"));
    WeaponComponent->SetupAttachment(RootComponent);

    ProtectionComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Protection"));
    ProtectionComponent->SetupAttachment(RootComponent);

    SupportComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Support"));
    SupportComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APirate::BeginPlay()
{
	Super::BeginPlay();
    Tags.Add(Tags::Pirate);
	
	BuildShip();

	if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
        // init hit callback
        auto lb_init = [&](USpacelInstancedMeshComponent* _component)
        {
            if (_component != nullptr)
            {
                _component->OnComponentHit.AddDynamic(this, &APirate::OnComponentsHit);
            }
        };

        lb_init(this->WeaponComponent);
        lb_init(this->ProtectionComponent);
        lb_init(this->SupportComponent);
        
        if (this->RedCube != nullptr)
        {
            this->RedCube->OnComponentHit.AddDynamic(this, &APirate::OnComponentsHit);
        }
	}
}

// Called every frame
void APirate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APirate::OnComponentsHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    auto lb = [&_hitComp, &_hit](USpacelInstancedMeshComponent* _component) -> bool
    {
        if (_component != nullptr && _component->GetUniqueID() == _hitComp->GetUniqueID())
        {
            FTransform out;
            _component->GetInstanceTransform(_hit.Item, out);
            return _component->Remove(out.GetLocation()) != -1;
        }
        return false;
    };

    bool ret { false };
    ret |= lb(this->WeaponComponent);
    ret |= lb(this->ProtectionComponent);
    ret |= lb(this->SupportComponent);

    if (!ret)
    {
        if (this->RedCube != nullptr && this->RedCube->GetUniqueID() == _hitComp->GetUniqueID())
        {
            if (ATeamActor* teamActor = Cast<ATeamActor>(_otherActor))
            {
                OnKilledDelegate.broadcast(teamActor->R_Team);
                this->Destroy();
            }
        }
    }
}

void APirate::BuildShip()
{
    auto lb_call = [](USpacelInstancedMeshComponent* _component)
    {
        if (_component != nullptr)
        {
            _component->Read();
            _component->InitLocations(true);
            _component->resetBuild();
        }
    };

    lb_call(this->WeaponComponent);
    lb_call(this->ProtectionComponent);
    lb_call(this->SupportComponent);
}

void APirate::Destroyed()
{
    BP_OnDestroy();
    Super::Destroyed();
}