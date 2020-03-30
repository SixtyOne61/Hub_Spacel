// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hub_Spacel/Source/Engine/Location/ChainedLocation.h"
#include "EnvironmentManager.generated.h"

UCLASS()
class HUB_SPACEL_API AEnvironmentManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnvironmentManager();

	// -- init actor, call on spawn
	void init(FVector2D const& _bornX, FVector2D const& _bornY, FVector2D const& _bornZ, int _cubeSize);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void createProceduralWorld();
	TSharedPtr<ChainedLocation> createChain(FVector& _location, TArray<FVector>& _openList);
	void addNeighboor(TArray<FVector> & _openList, FVector _location, EFace _where, TSharedPtr<ChainedLocation> _chain, EFace _inverse);

	// -- spawn asteroid
	void addProceduralMesh();

	// -- check if we have a valid noise in this points
	bool isValidNoise(int _x, int _y, int _z) const;
	bool isValidNoise(FVector const& _location) const;

	// -- check if we are on born
	bool isValidLocation(FVector const& _location) const;

	// -- check if this location is in m_currentObject
	TSharedPtr<ChainedLocation> isKnownLocation(FVector const& _location) const;

public:
    /* Material for asteroid */
    UPROPERTY(Category = "Material", EditAnywhere, BlueprintReadWrite)
    class UMaterialInstance* MatAsteroid = nullptr;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector2D m_bornX;
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector2D m_bornY;
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector2D m_bornZ;
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	int m_cubeSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TArray<class USpacelProceduralMeshComponent*> m_proceduralMeshComponents;

private:
    TArray<TSharedPtr<ChainedLocation>> m_currentObject;
};
