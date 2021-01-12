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

	// Called when the game starts or when spawned
	void BeginPlay() override;

	// Called every frame
	void Tick(float _deltaTime) override;

	int hit(FHitResult const& _hit, FString const& _team);
	void spawnMatiere(FVector const& _location, FString const& _team);

private:
	UFUNCTION()
	void OnRep_Instance();

public:
	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
	class UInstancedStaticMeshComponent* Matieres { nullptr };

private:
	TMap<FVector, FString> m_spawnByTeam {};

	TSet<int32> m_instanceToRemove {};

	UPROPERTY(ReplicatedUsing = "OnRep_Instance")
	TArray<FVector> RU_Instance{};
};
