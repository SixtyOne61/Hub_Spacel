// Fill out your copyright notice in the Description page of Project Settings.


#include "FireComponent.h"
#include "Player/ShipPawn.h"
#include "DataAsset/PlayerDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values for this component's properties
UFireComponent::UFireComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UFireComponent::BeginPlay()
{
	Super::BeginPlay();

	m_shipPawnOwner = MakeWeakObjectPtr(Cast<AShipPawn>(this->GetOwner()));
}

// Called every frame
void UFireComponent::TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction)
{
	Super::TickComponent(_deltaTime, _tickType, _thisTickFunction);

	if (!m_shipPawnOwner.IsValid())
	{
		return;
	}

	if (!ensure(m_shipPawnOwner.Get()->PlayerDataAsset != nullptr)) return;
	if (!ensure(m_shipPawnOwner.Get()->PlayerDataAsset->BulletClass != nullptr)) return;
	if (!ensure(m_shipPawnOwner.Get()->WeaponMeshComponent != nullptr)) return;

	UWorld* world { this->GetWorld() };
	if (!ensure(world != nullptr)) return;

    // check if we have boolean for fire (only set on server)
    if (m_isFire.hasValue() && m_isFire.value() && m_fireCountDown <= 0.0f)
    {
        FTransform transform{};
        m_shipPawnOwner.Get()->WeaponMeshComponent->GetInstanceTransform(m_fireIndex, transform, true);
        // reset scale
        transform.SetScale3D({ 1.0f, 1.0f, 1.0f });

        ++m_fireIndex;
        if (m_fireIndex >= m_shipPawnOwner.Get()->WeaponMeshComponent->GetInstanceCount())
        {
            m_fireIndex = 0;
        }

        if (m_target != nullptr && !m_target->IsPendingKill())
        {
            FVector pawnDir = m_shipPawnOwner.Get()->GetActorForwardVector();
            pawnDir.Normalize();
            FVector targetDir = UKismetMathLibrary::FindLookAtRotation(transform.GetLocation(), m_target->GetActorLocation()).Vector();
            targetDir.Normalize();

            float angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(pawnDir, targetDir)));
            if (angle <= 30)
            {
                transform.SetRotation(targetDir.ToOrientationQuat());
            }
        }

        AActor* laser = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(world, m_shipPawnOwner.Get()->PlayerDataAsset->BulletClass, transform));
        if (laser)
        {
            // init bullet
            laser->SetReplicates(true);
            laser->SetReplicateMovement(true);
            UGameplayStatics::FinishSpawningActor(laser, transform);
            if (UProjectileMovementComponent* comp = Cast<UProjectileMovementComponent>(laser->GetComponentByClass(UProjectileMovementComponent::StaticClass())))
            {
                FVector dir { FVector{1.0f, 0.0f, .0f }};
                comp->SetVelocityInLocalSpace(dir * comp->InitialSpeed);
            }
        }

        // reset count down
        m_fireCountDown = m_shipPawnOwner.Get()->PlayerDataAsset->TimeBetweenFire;
    }
    else if (m_fireCountDown != 0.0f)
    {
        // we can't use timer manager here, because we want to keep timer when we release trigger
        // if player spam trigger and use timer manager, we will just spam the first tick of the handle timer
        // and throw many bullet
        m_fireCountDown -= _deltaTime;
    }
}