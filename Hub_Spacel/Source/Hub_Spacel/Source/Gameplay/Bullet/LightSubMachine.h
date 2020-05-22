// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Source/Gameplay/Bullet/DefaultSubMachine.h"
#include "LightSubMachine.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ALightSubMachine : public ADefaultSubMachine
{
	GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ALightSubMachine();
	
public:
    UPROPERTY(Category = "Spawner", VisibleAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* BulletSpawner1 = nullptr;
    UPROPERTY(Category = "Spawner", VisibleAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* BulletSpawner2 = nullptr;
    UPROPERTY(Category = "Spawner", VisibleAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* BulletSpawner3 = nullptr;
    UPROPERTY(Category = "Spawner", VisibleAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* BulletSpawner4 = nullptr;
    UPROPERTY(Category = "Spawner", VisibleAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* BulletSpawner5 = nullptr;
    UPROPERTY(Category = "Spawner", VisibleAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* BulletSpawner6 = nullptr;
    UPROPERTY(Category = "Spawner", VisibleAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* BulletSpawner7 = nullptr;
};
