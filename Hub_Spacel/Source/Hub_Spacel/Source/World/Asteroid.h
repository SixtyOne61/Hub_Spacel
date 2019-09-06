// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hub_Spacel/Source/Engine/Location/ChainedLocation.h"
#include "Asteroid.generated.h"

class USpacelProceduralMeshComponent;
UCLASS()
class HUB_SPACEL_API AAsteroid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAsteroid();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// -- get / set
	void setEdges(TArray<TSharedPtr<ChainedLocation>>&& _edges);
	void setCubeSize(int _cubeSize);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	USpacelProceduralMeshComponent * m_proceduralMeshComponent;
};
