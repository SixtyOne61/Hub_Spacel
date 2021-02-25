// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <functional>
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
	void populate(class UInstancedStaticMeshComponent* _comp, TArray<FVector> const& _loc);
	int depopulate(class UInstancedStaticMeshComponent* _comp, FHitResult const& _hit, TSet<int32> & _index);

	UFUNCTION()
	void OnRep_InstanceTeam1();

	UFUNCTION()
	void OnRep_InstanceTeam2();

	UFUNCTION()
	void OnRep_InstanceTeam3();

	UFUNCTION()
	void OnRep_InstanceTeam4();

	UFUNCTION()
	void OnRep_InstanceTeam5();

	UFUNCTION()
	void OnRep_InstanceTeam6();

public:
	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
	class UInstancedStaticMeshComponent* MatieresTeam1 { nullptr };

	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
	class UInstancedStaticMeshComponent* MatieresTeam2 { nullptr };

	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
	class UInstancedStaticMeshComponent* MatieresTeam3 { nullptr };

	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
	class UInstancedStaticMeshComponent* MatieresTeam4 { nullptr };

	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
	class UInstancedStaticMeshComponent* MatieresTeam5 { nullptr };

	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
	class UInstancedStaticMeshComponent* MatieresTeam6 { nullptr };

	/* for client only */
	UPROPERTY()
	FString LocalTeam {};

private:
	UPROPERTY(ReplicatedUsing = "OnRep_InstanceTeam1")
	TArray<FVector> RU_InstanceTeam1{};
	TSet<int32> m_toRemoveTeam1 {};

	UPROPERTY(ReplicatedUsing = "OnRep_InstanceTeam2")
	TArray<FVector> RU_InstanceTeam2{};
	TSet<int32> m_toRemoveTeam2{};

	UPROPERTY(ReplicatedUsing = "OnRep_InstanceTeam3")
	TArray<FVector> RU_InstanceTeam3{};
	TSet<int32> m_toRemoveTeam3{};

	UPROPERTY(ReplicatedUsing = "OnRep_InstanceTeam4")
	TArray<FVector> RU_InstanceTeam4{};
	TSet<int32> m_toRemoveTeam4{};

	UPROPERTY(ReplicatedUsing = "OnRep_InstanceTeam5")
	TArray<FVector> RU_InstanceTeam5{};
	TSet<int32> m_toRemoveTeam5{};

	UPROPERTY(ReplicatedUsing = "OnRep_InstanceTeam6")
	TArray<FVector> RU_InstanceTeam6{};
	TSet<int32> m_toRemoveTeam6{};
};
