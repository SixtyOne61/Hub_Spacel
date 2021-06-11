// Fill out your copyright notice in the Description page of Project Settings.


#include "FireComponent.h"
#include "Player/ShipPawn.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ModuleComponent.h"
#include "DataAsset/PlayerDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Gameplay/Bullet/Missile.h"
#include "Gameplay/Bullet/Katyusha.h"

// Sets default values for this component's properties
UFireComponent::UFireComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called every frame
void UFireComponent::TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction)
{
	Super::TickComponent(_deltaTime, _tickType, _thisTickFunction);

    if (this->GetNetMode() != ENetMode::NM_DedicatedServer) return;

	if (get() == nullptr && !initShipPawnOwner()) return;

	if (!ensure(get()->PlayerDataAsset != nullptr)) return;
	if (!ensure(get()->PlayerDataAsset->BulletClass != nullptr)) return;
	if (!ensure(get()->ModuleComponent != nullptr)) return;
    if (!ensure(get()->ModuleComponent->WeaponMeshComponent != nullptr)) return;

	UWorld* world { this->GetWorld() };
	if (!ensure(world != nullptr)) return;

    // check if we have boolean for fire (only set on server)
    if (m_isFire.hasValue() && m_isFire.value() && m_fireCountDown <= 0.0f)
    {
        FTransform transform{};
        get()->ModuleComponent->WeaponMeshComponent->GetInstanceTransform(m_fireIndex, transform, true);
        // reset scale
        transform.SetScale3D({ 1.0f, 1.0f, 1.0f });

        ++m_fireIndex;
        if (m_fireIndex >= get()->ModuleComponent->WeaponMeshComponent->GetInstanceCount())
        {
            m_fireIndex = 0;
        }

        FVector bulletDir = UKismetMathLibrary::FindLookAtRotation(transform.GetLocation(), get()->TargetLocation).Vector();
        bulletDir.Normalize();
        transform.SetRotation(bulletDir.ToOrientationQuat());
        spawnBullet(transform);

        // reset count down
        if (ASpacelPlayerState* spacelPlayerState = get()->GetPlayerState<ASpacelPlayerState>())
        {
            uint8 lowSkillId = spacelPlayerState->getSkillId(ESkillType::Low);
            float coef = lowSkillId == (uint8)ESkill::FireRate ? get()->PlayerDataAsset->ReduceTimeBetweenFireWithLevel : 1.0f;
            if (get()->hasEffect(EEffect::MetaFormAttack))
            {
                coef = get()->PlayerDataAsset->ReduceTimeBetweenFireWithMetaForm;
            }
            m_fireCountDown = get()->PlayerDataAsset->TimeBetweenFire * coef;
        }
        else
        {
            ensure(true);
        }
    }
    else if (m_fireCountDown != 0.0f)
    {
        // we can't use timer manager here, because we want to keep timer when we release trigger
        // if player spam trigger and use timer manager, we will just spam the first tick of the handle timer
        // and throw many bullet
        m_fireCountDown -= _deltaTime;
    }
}

void UFireComponent::spawnBullet(FTransform const& _transform) const
{
    AActor* actor = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this->GetWorld(), get()->PlayerDataAsset->BulletClass, _transform));
    if (AProjectileBase* laser = Cast<AProjectileBase>(actor))
    {
        // init bullet
        laser->R_Team = get()->Team;
        UGameplayStatics::FinishSpawningActor(laser, _transform);
        setupProjectile(laser);

        if (AShipPawn* shipPawn = get<AShipPawn>())
        {
            shipPawn->RPCNetMulticastFxFireBullet();
        }
    }
}

void UFireComponent::launchMissile(FTransform const _transform) const
{
    AActor* actor = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this->GetWorld(), get()->PlayerDataAsset->MissileClass, _transform));
    if (AMissile* missile = Cast<AMissile>(actor))
    {
        missile->Target = m_target;
        missile->RPCNetMulticastTarget(m_target->GetFName());

        missile->R_Team = get()->Team;
        if (get<AShipPawn>() != nullptr)
        {
            get<AShipPawn>()->OnAddEffectDelegate.AddDynamic(missile, &AMissile::OnTargetEffect);
        }

        UGameplayStatics::FinishSpawningActor(missile, _transform);
        setupProjectile(missile);

        if (AShipPawn* shipPawn = get<AShipPawn>())
        {
            shipPawn->RPCNetMulticastFxFireMissile();
        }
    }
}

void UFireComponent::BeginPlay()
{
    Super::BeginPlay();
    if (this->GetNetMode() != ENetMode::NM_DedicatedServer) return;

    if (AShipPawn* shipPawn = get<AShipPawn>())
    {
        shipPawn->BP_InitFireComponent();
    }
}

void UFireComponent::spawnKatyusha()
{
    if (this->GetNetMode() != ENetMode::NM_DedicatedServer) return;

    m_nbKatyusha = DummyKatyushaLocations.Num() - 1;
    if (m_nbKatyusha < DummyKatyushaLocations.Num() && DummyKatyushaLocations[m_nbKatyusha] != nullptr)
    {
        m_nextKatyushaTransform = DummyKatyushaLocations[m_nbKatyusha]->GetComponentTransform();
        SpawnKatyusha();
    }
}

void UFireComponent::SpawnKatyusha()
{
    if (AShipPawn* shipPawn = get<AShipPawn>())
    {
        if (UPlayerDataAsset* dataAsset = shipPawn->PlayerDataAsset)
        {
            AActor* actor = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this->GetWorld(), dataAsset->KatyushaClass, m_nextKatyushaTransform));
            if (AKatyusha* katyusha = Cast<AKatyusha>(actor))
            {
                katyusha->R_TargetLocation = m_target->GetActorLocation();
                katyusha->R_Team = shipPawn->Team;

                UGameplayStatics::FinishSpawningActor(katyusha, m_nextKatyushaTransform);
                setupProjectile(katyusha);

                --m_nbKatyusha;
                if (m_nbKatyusha >= 0 && m_nbKatyusha < DummyKatyushaLocations.Num() && DummyKatyushaLocations[m_nbKatyusha] != nullptr)
                {
                    m_nextKatyushaTransform = DummyKatyushaLocations[m_nbKatyusha]->GetComponentTransform();

                    FTimerHandle handle;
                    this->GetWorld()->GetTimerManager().SetTimer(handle, this, &UFireComponent::SpawnKatyusha, 0.2f, false);
                }
            }
        }
    }
}

void UFireComponent::setupProjectile(AActor* _projectile) const
{
    if (UProjectileMovementComponent* comp = Cast<UProjectileMovementComponent>(_projectile->GetComponentByClass(UProjectileMovementComponent::StaticClass())))
    {
        FVector dir{ FVector{ 1.0f, 0.0f, .0f } };
        comp->SetVelocityInLocalSpace(dir * comp->InitialSpeed);
    }

    FString profile = "P" + get()->Team.ToString();
    profile = profile.Replace(TEXT(" "), TEXT(""));
    if (USphereComponent* comp = Cast<USphereComponent>(_projectile->GetComponentByClass(USphereComponent::StaticClass())))
    {
        comp->SetCollisionProfileName(*profile);
    }

    if (UInstancedStaticMeshComponent* comp = Cast<UInstancedStaticMeshComponent>(_projectile->GetComponentByClass(UInstancedStaticMeshComponent::StaticClass())))
    {
        comp->SetCollisionProfileName(*profile);
    }

    FString tag = "Team:" + get()->Team.ToString();
    _projectile->Tags.Add(*tag);


    if (ASpacelPlayerState* spacelPlayerState = get()->GetPlayerState<ASpacelPlayerState>())
    {
        if (AProjectileBase* projectileBase = Cast<AProjectileBase>(_projectile))
        {
            projectileBase->R_PlayerIdOwner = spacelPlayerState->PlayerId;
        }
    }
}