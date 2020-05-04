// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hub_Spacel/Source/Engine/Location/ChainedLocation.h"
#include "EnvironmentManager.generated.h"

// only for c++
struct CoordInfo
{
    float m_noiseValue = 0.0f;
    bool m_use = 0.0f;

    TSharedPtr<ChainedLocation> m_chainedLocation = nullptr;

    TMap<EFace, int> m_neighboor = { {EFace::Back, -1}, {EFace::Front, -1},
                                            {EFace::Bot, -1}, {EFace::Top, -1},
                                            {EFace::Right, -1}, {EFace::Left, -1} };

    inline bool isValid() const
    {
        return m_noiseValue > 0.75f && !m_use;
    }
};

UCLASS()
class HUB_SPACEL_API AEnvironmentManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnvironmentManager();

	// -- init actor, call on spawn
	void init(FVector2D const& _bornX, FVector2D const& _bornY, FVector2D const& _bornZ, FVector const& _cubeSize);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void createProceduralWorld();
	TSharedPtr<ChainedLocation> createChain(FVector const& _location, TArray<FVector>& _openList);
	void addNeighboor(TArray<FVector> & _openList, FVector _location, EFace _where, TSharedPtr<ChainedLocation> _chain, EFace _inverse);
    void addNeighboor(CoordInfo & _info, TArray<CoordInfo> & _list);

	// -- spawn asteroid
	void addProceduralMesh();

	// -- check if we have a valid noise in this points
	bool isValidNoise(FVector const& _location) const;

	// -- check if we are on born
	bool isValidLocation(FVector const& _location) const;

	// -- check if this location is in m_currentObject
	TSharedPtr<ChainedLocation> isKnownLocation(FVector const& _location) const;

    /* get noise value */
    float getNoise(FVector const& _location) const;

public:
    /* Material for asteroid */
    UPROPERTY(Category = "Material", EditAnywhere, BlueprintReadWrite)
    class UMaterialInstance* MatAsteroid = nullptr;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector2D BornX;
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector2D BornY;
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector2D BornZ;
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector CubeSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	TArray<class USpacelProceduralMeshComponent*> m_proceduralMeshComponents;

private:
    TArray<TSharedPtr<ChainedLocation>> m_currentObject;
};
