// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Mission/MissionActor.h"
#include "Util/SpacelEvent.h"
#include "Pirate.generated.h"

UCLASS()
class HUB_SPACEL_API APirate : public AMissionActor
{
	GENERATED_BODY()
	
	friend class MissionPirate;

public:
	// Sets default values for this actor's properties
	APirate();

	// Called every frame
	void Tick(float _deltaTime) override;

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;
	void Destroyed() override;

	UFUNCTION()
	void OnComponentsHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnDestroy();

private:
	/* call on begin play for build station */
	UFUNCTION(BlueprintCallable)
	void BuildShip();

	/* find player in world, call on begin play and register them */
	void registerPlayers();

	/* call on tick and check if we can fire on target */
	void fire();

	/* spawn bullet actor tower this direction */
	void spawnBullet(FVector const& _startLocation, FVector const& _targetLocation) const;

protected:
	UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USpacelInstancedMeshComponent* WeaponComponent { nullptr };

	UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USpacelInstancedMeshComponent* TowerComponent { nullptr };

	UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USpacelInstancedMeshComponent* BaseComponent{ nullptr };

	UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USpacelInstancedMeshComponent* CircleComponent{ nullptr };

	UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USpacelInstancedMeshComponent* SupportComponent { nullptr };

	UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USpacelInstancedMeshComponent* RedCube { nullptr };

	Util::Event<FName const&> OnKilledDelegate { };

	/* constante fire rate */
	UPROPERTY(Category = "Component|Timer", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float FireRate { 0.3f };

	/* delay before first fire */
	UPROPERTY(Category = "Component|Timer", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float FirstDelay { 2.5f };

	UPROPERTY(Category = "Component|Setup", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float MaxDistance { 5000.0f };

	/* bullet class for fire */
	UPROPERTY(Category = "Component|Subclass", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AActor> BulletClass;

private:
	/* list of all players */
	TArray<AActor*> m_players {};

	/* timer between fire */
	float m_timer { 0.0f };

	/* index for next fire location */
	int m_fireIndex { 0 };
};
