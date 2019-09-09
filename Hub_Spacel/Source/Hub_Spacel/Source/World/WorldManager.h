// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldManager.generated.h"

UCLASS()
class HUB_SPACEL_API AWorldManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// -- spawn chunck environment
	void spawnChunckEnvironment();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	int m_nbChunck;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	int m_chunckSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	int m_cubeSize;

	UPROPERTY(EditDefaultsOnly, Category = "Procedural")
	TSubclassOf<class AEnvironmentManager> BP_environment;

};
