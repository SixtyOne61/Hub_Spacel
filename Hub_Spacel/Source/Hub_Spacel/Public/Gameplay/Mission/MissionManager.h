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
	void OnAskMission(EMission _missionId);

	void startMission(FMission const& _mission) const;
	void endMission(FMission const& _mission) const;

public:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UMissionDataAsset* MissionDataAsset{ nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UEditorHackDataAsset* HackDataAsset{ nullptr };

	TArray<TUniquePtr<MissionBehaviour>> m_openMission;
	TArray<TUniquePtr<MissionSilence>> m_silenceMission;
};
