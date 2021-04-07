// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PointUserWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnimEnd, UPointUserWidget*, _widget);

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UPointUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void StartAnim(FString const& _value);

	UFUNCTION(BlueprintCallable)
	void EndAnim() { OnAnimEndDelegate.Broadcast(this) ; }

public:
	UPROPERTY(VisibleAnywhere, BlueprintAssignable, Category = "EventDispatchers")
	FOnAnimEnd OnAnimEndDelegate {};
};
