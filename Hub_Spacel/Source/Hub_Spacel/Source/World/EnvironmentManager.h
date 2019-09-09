// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hub_Spacel/Source/Engine/Location/ChainedLocation.h"
#include "Hub_Spacel/Source/Noise/SpacelNoise.h"
#include "EnvironmentManager.generated.h"

UCLASS()
class HUB_SPACEL_API AEnvironmentManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnvironmentManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void createProceduralWorld();
	TSharedPtr<ChainedLocation> createChain(FVector& _location, TArray<FVector>& _openList);
	void addNeighboor(TArray<FVector> & _openList, FVector _location, EFace _where, TSharedPtr<ChainedLocation> _chain, EFace _inverse);

	// -- spawn asteroid
	void spawnAsteroid();

	// -- check if we have a valid noise in this points
	bool isValidNoise(int _x, int _y, int _z) const;
	bool isValidNoise(FVector const& _location) const;

	// -- check if we are on born
	bool isValidLocation(FVector const& _location) const;

	// -- check if this location is in m_currentObject
	TSharedPtr<ChainedLocation> isKnownLocation(FVector const& _location) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	int m_bornX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	int m_bornY;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	int m_bornZ;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	int m_cubeSize;

	UPROPERTY(EditDefaultsOnly, Category = "Procedural Type")
	TSubclassOf<class AAsteroid> BP_asteroid;

	// test TO DO : create a singleton
	SpacelNoise m_noise;

private:
	TArray<TSharedPtr<ChainedLocation>> m_currentObject;
};
