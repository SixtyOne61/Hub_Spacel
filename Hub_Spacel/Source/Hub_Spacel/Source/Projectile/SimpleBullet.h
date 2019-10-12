// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleBullet.generated.h"

UCLASS()
class HUB_SPACEL_API ASimpleBullet : public AActor
{
	GENERATED_BODY()

protected:
	/** StaticMesh component that will be the visuals for our flying pawn */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* SimpleBulletMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup")
	int ForceOnStart = 100000;
	
public:	
	// Sets default values for this actor's properties
	ASimpleBullet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// -- get / set
	inline FVector const& getLaunchForward() const { return m_launchVector; }

	void launchBullet(FVector _forward);

protected:
	FVector m_launchVector = FVector::ZeroVector;
};
