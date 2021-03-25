// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FogActor.generated.h"

UCLASS()
class HUB_SPACEL_API AFogActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFogActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBeginOverlap(class UPrimitiveComponent* _comp, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, int32 _otherBodyIndex, bool _bFromSweep, const FHitResult& _sweepResult);

	UFUNCTION()
	void OnEndOverlap(class UPrimitiveComponent* _comp, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, int32 _otherBodyIndex);

private:
	void onOverlap(class AActor* _otherActor, bool _value) const;

protected:
	UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
	class UNiagaraComponent* FogNiagaraSystem { nullptr };

	UPROPERTY(Category = "Component", VisibleAnywhere)
	class UBoxComponent* BoxComponent { nullptr };
};
