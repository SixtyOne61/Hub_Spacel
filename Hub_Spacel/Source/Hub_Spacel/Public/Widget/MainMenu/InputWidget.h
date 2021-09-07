// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "InputWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUpdateInput, EInput, _input, FKey, _key);

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UInputWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_Setup(FName _inputName, TArray<class UTexture2D*> const& _textures, class UTexture2D* _keyTexture, FKey const& _key);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateKeyTexture(class UTexture2D* _keyTexture);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateBindingAxis(FString const& _action, FKey const& _key, float _value);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateBindingAction(FString const& _action, FKey const& _key);

	UFUNCTION(BlueprintCallable)
	void OnNewKeySelected(FKey const& _key);

public:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UKeyDataAsset* KeyDataAsset{ nullptr };

	// set on owner widget in editor
	UPROPERTY(Category = "Param", EditAnywhere, BlueprintReadWrite)
	EInput Type {};

	UPROPERTY(Category = "Param", BlueprintReadWrite)
	TArray<FString> ActionNames {};

	UPROPERTY(Category = "Param", BlueprintReadWrite)
	TArray<float> Values {};

	UPROPERTY(Category = "Param", BlueprintReadWrite)
	FKey Key {};

	UPROPERTY()
	FUpdateInput UpdateInputDelegate {};
};
