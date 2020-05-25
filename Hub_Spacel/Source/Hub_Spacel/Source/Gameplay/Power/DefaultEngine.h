// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DefaultEngine.generated.h"

UCLASS()
class HUB_SPACEL_API ADefaultEngine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADefaultEngine();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable)
    virtual bool GenerateMesh(TArray<FVector> const& _ignoreCoord);

public:
    UPROPERTY(Category = "Root", EditAnywhere, BlueprintReadWrite)
    class USceneComponent* Root = nullptr;
    UPROPERTY(Category = "Mesh", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpacelProceduralMeshComponent* ProceduralMesh = nullptr;
    UPROPERTY(Category = "Mesh", EditAnywhere, BlueprintReadWrite)
    class UMaterialInstance* Mat = nullptr;

};
