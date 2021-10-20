// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enum/SpacelEnum.h"
#include "PawnCapture.generated.h"

UCLASS()
class HUB_SPACEL_API APawnCapture : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APawnCapture();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float _deltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Build")
	void BuildShip(EFormType _type);

protected:
	UPROPERTY(Category = "Component|Root", VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* DriverMeshComponent { nullptr };

	UPROPERTY(Category = "Component|Root", VisibleAnywhere, BlueprintReadWrite)
	class UPoseableMeshComponent* BaseShipMeshComponent{ nullptr };

	UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite)
	class USpacelInstancedMeshComponent* WeaponComponent { nullptr };

	UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite)
	class USpacelInstancedMeshComponent* ProtectionComponent { nullptr };

	UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite)
	class USpacelInstancedMeshComponent* SupportComponent { nullptr };

	UPROPERTY(Category = "Component|Param", VisibleAnywhere, BlueprintReadWrite)
	float Time { 7.0f };

protected:
	float m_currentTime { 0.0f };
	EFormType m_currentForm { EFormType::Base };
};
