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

	/* spawn chunck environment */
	void spawnChunckEnvironment();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    class UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	uint8 NbChunckPerAxis = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    uint8 NbCubePerChunckPerAxis = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 CubeSize = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Procedural")
	TSubclassOf<class AEnvironmentManager> EnvironmentClass;
};
