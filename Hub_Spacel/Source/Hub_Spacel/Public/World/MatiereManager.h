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

	void spawnMatiere(FVector const& _location, FString const& _team);

private:
	UFUNCTION(Reliable, Client)
	void RPCAddMatiere(FVector const& _location);

	UFUNCTION()
	void OnRep_AddInstance();

public:
	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
	class UInstancedStaticMeshComponent* Matieres { nullptr };

private:
	TMap<FVector, FString> m_spawnByTeam {};

	UPROPERTY(ReplicatedUsing = "OnRep_AddInstance")
	FVector RU_AddInstance{};
};
