// Fill out your copyright notice in the Description page of Project Settings.


#include "Pirate.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Mesh/SpacelInstancedMeshComponent.h"
#include "Util/SimplyXml.h"
#include "Util/Tag.h"
#include "Util/DebugScreenMessage.h"
#include "Gameplay/Bullet/ProjectileBase.h"
#include "Player/Common/CommonPawn.h"

// Sets default values
APirate::APirate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RedCube = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("RedCube"));
	RootComponent = RedCube;

    WeaponComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Weapon"));
    WeaponComponent->SetupAttachment(RootComponent);

    TowerComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Tower"));
    TowerComponent->SetupAttachment(RootComponent);

    BaseComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Base"));
    BaseComponent->SetupAttachment(RootComponent);

    CircleComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Circle"));
    CircleComponent->SetupAttachment(RootComponent);

    SupportComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Support"));
    SupportComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APirate::BeginPlay()
{
	Super::BeginPlay();
    Tags.Add(Tags::Pirate);

	if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
        BuildShip();

        // init hit callback
        auto lb_init = [&](USpacelInstancedMeshComponent* _component)
        {
            if (_component != nullptr)
            {
                _component->OnComponentHit.AddDynamic(this, &APirate::OnComponentsHit);
            }
        };

        lb_init(this->TowerComponent);
        lb_init(this->BaseComponent);
        lb_init(this->CircleComponent);
        lb_init(this->SupportComponent);
        lb_init(this->RedCube);
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
            _component->RPCNetMulticastRemove(out.GetLocation());
            return true;
        }
        return false;
    };

    lb(this->TowerComponent);
    lb(this->BaseComponent);
    lb(this->CircleComponent);
    lb(this->SupportComponent);
    if (lb(this->RedCube))
    {
        // only bullet can kill station
        if (AProjectileBase* projectileBase = Cast<AProjectileBase>(_otherActor))
        {
            OnKilledDelegate.broadcast(projectileBase->R_Team);
            this->Destroy();
        }
    }
}

void APirate::BuildShip()
{
    auto lb_call = [](USpacelInstancedMeshComponent* _component)
    {
        if (_component != nullptr)
        {
            _component->UseForm(EFormType::Base, true);
        }
    };

    lb_call(this->WeaponComponent);
    lb_call(this->TowerComponent);
    lb_call(this->BaseComponent);
    lb_call(this->CircleComponent);
    lb_call(this->SupportComponent);
    lb_call(this->RedCube);
}

void APirate::Destroyed()
{
    BP_OnDestroy();
    Super::Destroyed();
}