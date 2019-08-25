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

	TSharedPtr<ChainedLocation> createChain(FVector& _location, TArray<FVector>& _openList) const;
	void createProceduralWorld() const;
	void addNeighboor(TArray<FVector> & _openList, FVector _location, EFace _where, TSharedPtr<ChainedLocation> _chain, EFace _inverse) const;

	bool isValidNoise(int _x, int _y, int _z) const;
	bool isValidNoise(FVector const& _location) const;

	// -- check if we are on born
	bool isValidLocation(FVector const& _location) const;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
	int m_bornX;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
	int m_bornY;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
	int m_bornZ;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
	int m_cubeSize;

	// test
	SpacelNoise m_noise;
};
