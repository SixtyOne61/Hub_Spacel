// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MatiereManager.generated.h"

UCLASS()
class HUB_SPACEL_API AMatiereManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMatiereManager();

	int hit(FHitResult const& _hit) { return 0 ; } // TO DO delay destruction

	void spawnMatiere(FVector const& _location, FString const& _team) {}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
