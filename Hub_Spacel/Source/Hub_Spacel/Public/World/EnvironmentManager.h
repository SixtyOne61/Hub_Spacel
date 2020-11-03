// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Mesh/LocationInformation.h"
#include <optional>
#include "EnvironmentManager.generated.h"

UCLASS()
class HUB_SPACEL_API AEnvironmentManager : public AActor
{
	GENERATED_BODY()

    friend class AWorldManager;
	
public:	
	// Sets default values for this actor's properties
	AEnvironmentManager();

private:
	/* init actor, call by World Manager, most of time on editor, we keep  */
	bool init(FVector2D const& _bornX, FVector2D const& _bornY, FVector2D const& _bornZ, FVector const& _cubeSize);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    /* full generate world */
	bool generateEnvironment();

    /* recurcive call */
    void findMeshPoint(FLocationInformation& _node, TArray<FLocationInformation> & _currentObject, TArray<FLocationInformation> & _list, int & _nbPoint);

    /* get noise value */
    float getNoise(FVector const& _location) const;

    /* create USpacelProceduralMeshComponent with m_currentObject information, then reset m_currentObject */
    void createProceduralMeshComponent(TArray<FLocationInformation> && _locations, int const& _nbPoint);

public:
    /* Material for asteroid */
	// TO DO change this material
    UPROPERTY(Category = "Material", EditAnywhere, BlueprintReadWrite)
    class UMaterialInstance* MatAsteroid = nullptr;
    UPROPERTY(Category = "Material", EditAnywhere, BlueprintReadWrite)
    int StencilValue = 0;
    UPROPERTY(Category = "Material", EditAnywhere, BlueprintReadWrite)
    float TricknessValue = 2.0f;

	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector2D BornX;
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector2D BornY;
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector2D BornZ;
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector CubeSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TArray<class USpacelProceduralMeshComponent*> ProceduralMeshComponents;
};
