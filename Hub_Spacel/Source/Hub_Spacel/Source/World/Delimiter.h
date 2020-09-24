// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Delimiter.generated.h"

UCLASS()
class HUB_SPACEL_API ADelimiter : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADelimiter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
    /* End overlap event */
    UFUNCTION()
    void OnEndOverlapEvent(class UPrimitiveComponent* _overlappedComp, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, int32 _otherBodyIndex);

public:
    UPROPERTY(Category = "Box", EditAnywhere, BlueprintReadWrite)
    class UBoxComponent* BoxComponent = nullptr;
};
