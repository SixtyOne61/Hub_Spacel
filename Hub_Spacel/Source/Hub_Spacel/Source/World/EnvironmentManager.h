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

	/* init actor, call on spawn */
	void Init(FVector2D const& _bornX, FVector2D const& _bornY, FVector2D const& _bornZ, FVector const& _cubeSize);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    /* full generate world */
	void generateEnvironment();

    /* recurcive call */
    void findMeshPoint(CoordInfo & _info, TArray<CoordInfo> & _list, FString & _xmlContent);

    /* get noise value */
    float getNoise(FVector const& _location) const;

    /* return true if an xml is found and he is valid */
    bool readXml();

    /* create USpacelProceduralMeshComponent with m_currentObject information, then reset m_currentObject */
    void createProceduralMeshComponent();

public:
    /* Material for asteroid */
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

private:
    TArray<TSharedPtr<ChainedLocation>> m_currentObject;
};
