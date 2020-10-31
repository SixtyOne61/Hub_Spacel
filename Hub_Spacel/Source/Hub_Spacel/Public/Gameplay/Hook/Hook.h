// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralActor.h"
#include "Hook.generated.h"

UCLASS()
class HUB_SPACEL_API AHook : public AProceduralActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHook();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
    virtual bool GenerateMesh(TArray<FVector> const& _ignoreCoord) override;

    /* End overlap event */
    UFUNCTION()
    void OnBeginOverlap(class UPrimitiveComponent* _overlappedComponent, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, int32 _otherBodyIndex, bool _bFromSweep, const FHitResult& _sweepResult);

public:
    UPROPERTY(Category = "Hook", EditAnywhere, BlueprintReadWrite)
    class UBoxComponent* BoxComponent = nullptr;
    UPROPERTY(Category = "Setup", EditAnywhere, BlueprintReadWrite)
    float InnerRadius = 0.0f;
};
