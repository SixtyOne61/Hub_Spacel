// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void BuildShip();

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USetupAttributeDataAsset* WeaponDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USetupAttributeDataAsset* ProtectionDataAsset{ nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USetupAttributeDataAsset* SupportDataAsset{ nullptr };

protected:
	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
	class UInstancedStaticMeshComponent* WeaponMeshComponent { nullptr };

	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
	class UInstancedStaticMeshComponent* ProtectionMeshComponent { nullptr };

	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
	class UInstancedStaticMeshComponent* SupportMeshComponent { nullptr };
};
