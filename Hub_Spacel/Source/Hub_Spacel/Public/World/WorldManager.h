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
	void BeginPlay() override;

	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* _overlappedComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, int32 _otherBodyIndex);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
    class UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	class UWorldDataAsset* WorldDataAsset { nullptr };

	UPROPERTY(EditAnywhere, Category = "Settings")
	TSubclassOf<class AChunck> ChunckClass { nullptr };
};
