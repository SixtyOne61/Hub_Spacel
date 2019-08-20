// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Asteroid.generated.h"

class UProceduralMeshComponent;
UCLASS()
class HUB_SPACEL_API AAsteroid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAsteroid();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called when actor is spawned (at runtime or when you drop it into the world in editor)
	virtual void PostActorCreated() override;
	// Called when actor is already in level and map is opened
	virtual void PostLoad() override;

private:
	// -- generate procedural mesh
	void generateMesh();

protected:
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent * m_meshComponent;
};
