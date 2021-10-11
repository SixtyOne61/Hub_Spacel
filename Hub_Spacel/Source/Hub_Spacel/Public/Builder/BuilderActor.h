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
	void BeginPlay() override;

public:	
	// Called every frame
	void Tick(float DeltaTime) override;

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

	template<class T>
	void createComponent(T *& _component, FName&& _name)
	{
		_component = CreateDefaultSubobject<T>(_name);
		if (!ensure(_component != nullptr)) return;

		_component->SetRenderCustomDepth(true);
		_component->SetupAttachment(RootComponent);
	}

protected:
	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* RedCubeComponent { nullptr };

	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UXmlInstancedStaticMeshComponent* WeaponComponent { nullptr };

	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UXmlInstancedStaticMeshComponent* ProtectionComponent { nullptr };

	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UXmlInstancedStaticMeshComponent* EngineComponent { nullptr };
};
