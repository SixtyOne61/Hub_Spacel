// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataAsset/MissionDataAsset.h"
#include "Gameplay/Mission/MissionBehaviour.h"
#include "MissionManager.generated.h"

UCLASS()
class HUB_SPACEL_API AMissionManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMissionManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void OnStartGame();

	void startMissionOnAllClient(FMission const& _mission) const;
	void endMissionOnNetMulticast(FMission const& _mission) const;

	void startMissionComet();

public:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UMissionDataAsset* MissionDataAsset{ nullptr };

	UPROPERTY(Category = "Sub class", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AComet> CometClass { nullptr };

	TArray<TUniquePtr<MissionBehaviour>> m_openMission;
	TArray<TUniquePtr<MissionSilence>> m_silenceMission;
};
