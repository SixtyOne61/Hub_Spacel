// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerActorComponent.h"
#include "Enum/SpacelEnum.h"
#include <functional>
#include "RepairComponent.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API URepairComponent : public UPlayerActorComponent
{
	GENERATED_BODY()

	friend class AShipPawn;

public:
	URepairComponent();

	void BeginPlay() override;

public:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UUniqueSkillDataAsset* RepairSkillDataAsset { nullptr };
	
private:
	UFUNCTION()
	void OnUpdateMatiere(int _value, EMatiereOrigin _type);

	/* repair generic */
	ESkillReturn onRepair();

	/* force repair */
	void heal(uint8 _value);
};
