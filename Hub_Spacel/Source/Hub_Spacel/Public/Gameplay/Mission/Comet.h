// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Mission/MissionActor.h"
#include "Util/SpacelEvent.h"
#include "Comet.generated.h"

UCLASS()
class HUB_SPACEL_API AComet : public AMissionActor
{
	GENERATED_BODY()

	friend class AMissionManager;
	
public:	
	// Sets default values for this actor's properties
	AComet();

	void hit(class AShipPawn* _pawn, FString const& _team) override;

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;
	void Tick(float _deltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	class UStaticMeshComponent* MeshComponent { nullptr };

	UPROPERTY(EditAnywhere, Category = "Default")
	class UProjectileMovementComponent* ProjectileMovementComponent{ nullptr };

	Util::Event<FString const&> m_onIntercep {};

	int32 m_speedRoll { 0 };
};
