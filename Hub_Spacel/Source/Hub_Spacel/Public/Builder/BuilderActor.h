// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enum/SpacelEnum.h"
#include "BuilderActor.generated.h"

UCLASS()
class HUB_SPACEL_API ABuilderActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuilderActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable)
	void Export() const;

	UFUNCTION(BlueprintCallable)
	void Add(EBuilderType _type, FVector const& _location);

	UFUNCTION(BlueprintCallable)
	void Remove(EBuilderType _type, FVector const& _location);

protected:
	/* return component for a type */
	class UXmlInstancedStaticMeshComponent* get(EBuilderType _type) const;

protected:
	UPROPERTY(Category = "Component", EditAnywhere, BlueprintReadWrite)
	class UXmlInstancedStaticMeshComponent* WeaponComponent { nullptr };

	UPROPERTY(Category = "Component", EditAnywhere, BlueprintReadWrite)
	class UXmlInstancedStaticMeshComponent* ProtectionComponent{ nullptr };

	UPROPERTY(Category = "Component", EditAnywhere, BlueprintReadWrite)
	class UXmlInstancedStaticMeshComponent* EngineComponent{ nullptr };
};
