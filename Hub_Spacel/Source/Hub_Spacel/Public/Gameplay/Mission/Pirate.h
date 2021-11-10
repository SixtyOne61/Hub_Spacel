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
	void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;
	void Destroyed() override;

	void BuildShip();

	UFUNCTION()
	void OnComponentsHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnDestroy();

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
};
