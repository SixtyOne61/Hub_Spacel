// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralActor.generated.h"

UCLASS()
class HUB_SPACEL_API AProceduralActor : public AActor
{
    GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AProceduralActor();

    UFUNCTION(BlueprintCallable)
    virtual bool GenerateMesh(TArray<FVector> const& _ignoreCoord);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
    UPROPERTY(Category = "Root", EditAnywhere, BlueprintReadWrite)
    class USceneComponent* Root = nullptr;
    UPROPERTY(Category = "Mesh", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpacelProceduralMeshComponent* ProceduralMesh = nullptr;
    UPROPERTY(Category = "Mesh", EditAnywhere, BlueprintReadWrite)
    class UMaterialInstance* Mat = nullptr;
};
