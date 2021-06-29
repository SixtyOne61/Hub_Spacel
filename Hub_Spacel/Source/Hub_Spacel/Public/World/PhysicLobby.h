// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enum/SpacelEnum.h"
#include "PhysicLobby.generated.h"

UCLASS()
class HUB_SPACEL_API APhysicLobby : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APhysicLobby();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnChangeState(EGameState _state);

	UFUNCTION()
	void SetupOwningTeam();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_TeamColor(FColor _color);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* Colors{ nullptr };
};
