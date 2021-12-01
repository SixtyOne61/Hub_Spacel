// Fill out your copyright notice in the Description page of Project Settings.


#include "Pirate.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Mesh/SpacelInstancedMeshComponent.h"
#include "Util/SimplyXml.h"
#include "Util/Tag.h"
#include "Util/DebugScreenMessage.h"
#include "Player/Common/CommonPawn.h"
#include "Gameplay/Bullet/ProjectileBase.h"
#include "GameState/SpacelGameState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetMathLibrary.h"

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

        registerPlayers();

        // init timer to first delay
        m_timer = this->FirstDelay;
	}
}

// Called every frame
void APirate::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        m_timer -= _deltaTime;
        if (m_timer <= 0.0f)
        {
            fire();
            m_timer = this->FireRate;
        }
    }
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

void APirate::registerPlayers()
{
    m_players.Empty();
    if (auto* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
    {
        auto const& players = spacelGameState->PlayerArray;
        for (auto* player : players)
        {
            if (player != nullptr)
            {
                m_players.Add(player->GetPawn());
            }
        }
    }
}

void APirate::fire()
{
    auto* world = this->GetWorld();

    // all check
    if(world == nullptr) return;
    if (this->WeaponComponent == nullptr) return;
    if(m_fireIndex >= this->WeaponComponent->GetInstanceCount()) return;

    FTransform out;
    this->WeaponComponent->GetInstanceTransform(m_fireIndex, out, true);
    FVector const& startLocation = out.GetLocation();

    UE_LOG(LogTemp, Warning, TEXT("start location: %s"), *startLocation.ToString());

    TArray<FVector> targets {};
    for (auto* act : m_players)
    {
        if (act != nullptr && !act->IsPendingKill())
        {
            FVector const& targetLocation { act->GetActorLocation() };
            // check distance with player
            if (FVector::Distance(startLocation, targetLocation) <= this->MaxDistance)
            {
                FHitResult hits;
                world->LineTraceSingleByChannel(hits, startLocation, targetLocation, ECollisionChannel::ECC_Pawn);
                if (hits.Actor.IsValid() && hits.Actor.Get()->ActorHasTag(Tags::Player))
                {
                    targets.Add(hits.Actor.Get()->GetActorLocation());
                }
            }
        }
    }

    // take a random target
    if (targets.Num() != 0)
    {
        int rand = FMath::RandRange(0, targets.Num() - 1);
        spawnBullet(startLocation, targets[rand]);
    }

    // next fire index
    if (this->WeaponComponent != nullptr)
    {
        ++m_fireIndex;
        if (m_fireIndex >= this->WeaponComponent->GetInstanceCount())
        {
            m_fireIndex = 0;
        }
    }
}

void APirate::spawnBullet(FVector const& _startLocation, FVector const& _targetLocation) const
{
    FTransform tr {};
    tr.SetLocation(_startLocation);

    FVector bulletDir = UKismetMathLibrary::FindLookAtRotation(_startLocation, _targetLocation).Vector();
    bulletDir.Normalize();
    tr.SetRotation(bulletDir.ToOrientationQuat());

    AActor* actor = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this->GetWorld(), this->BulletClass, tr));
    if (AProjectileBase* laser = Cast<AProjectileBase>(actor))
    {
        // init bullet
        laser->R_Team = "Team 4";
        UGameplayStatics::FinishSpawningActor(laser, tr);
        
        if (UProjectileMovementComponent* comp = Cast<UProjectileMovementComponent>(laser->GetComponentByClass(UProjectileMovementComponent::StaticClass())))
        {
            FVector dir{ FVector{ 1.0f, 0.0f, .0f } };
            comp->SetVelocityInLocalSpace(dir * comp->InitialSpeed);
        }

        FString profile = "PTeam4";
        if (USphereComponent* comp = Cast<USphereComponent>(laser->GetComponentByClass(USphereComponent::StaticClass())))
        {
            comp->SetCollisionProfileName(*profile);
        }

        if (UInstancedStaticMeshComponent* comp = Cast<UInstancedStaticMeshComponent>(laser->GetComponentByClass(UInstancedStaticMeshComponent::StaticClass())))
        {
            comp->SetCollisionProfileName(*profile);
        }

        FString tag = "Team:Team 4";
        laser->Tags.Add(*tag);
    }
}