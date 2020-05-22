// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DefaultSubMachine.generated.h"

UCLASS()
class HUB_SPACEL_API ADefaultSubMachine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADefaultSubMachine();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    /* return transform where spawn bullet */
    bool getWoldLocationBulletSpawner(FVector & _out);

public:
    UPROPERTY(Category = "Spawner", EditAnywhere, BlueprintReadWrite)
    class USceneComponent* Root = nullptr;

    UPROPERTY(Category = "Spawner", VisibleAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* DefaultBulletSpawner = nullptr;

protected:
    /* list of all bullet spawner */
    TArray<class UStaticMeshComponent*> m_bulletSpawners;

    /* id of next spawner */
    uint8 m_idBulletSpawner = 0;
};
