// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HookComponent.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API AHookComponent : public AActor
{
	GENERATED_BODY()
	
public:
    AHookComponent();

protected:
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable)
    virtual bool GenerateHook(float _innerRadius);

public:
    UPROPERTY(Category = "Root", EditAnywhere, BlueprintReadWrite)
    class USceneComponent* Root = nullptr;
    UPROPERTY(Category = "Mesh", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpacelProceduralMeshComponent* ProceduralMesh = nullptr;
    UPROPERTY(Category = "Mesh", EditAnywhere, BlueprintReadWrite)
    class UMaterialInstance* Mat = nullptr;
    UPROPERTY(Category = "Hook", EditAnywhere, BlueprintReadWrite)
    class UBoxComponent* BoxComponent = nullptr;
};
