// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "SkillItemWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnChooseSkill, ESkill, _id, FString const&, _desc, ESkillType, _type, UTexture2D*, _icon, FSlateColor const&, _color);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHoverSkill, ESkillType, _type, FString const&, _title, FString const&, _desc, FSlateColor const&, _color);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnHoverSkill, ESkillType, _type);

USTRUCT(BlueprintType)
struct FSkillChooseData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkill Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor BackgroundColor{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Title{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Desc{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* Icon{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkillType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VerboseEffect {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* OnClickSound { nullptr };
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USkillItemWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "UI|Event")
	void OnChooseSkill();

	UFUNCTION(BlueprintCallable, Category = "UI|Event")
	void OnHover();

	UFUNCTION(BlueprintCallable, Category = "UI|Event")
	void OnUnHover();

private:
	UFUNCTION()
	void OnChangeState(EGameState _state);

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, BlueprintAssignable)
	FOnChooseSkill OnChooseSkillDelegate {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, BlueprintAssignable)
	FOnHoverSkill OnHoverSkillDelegate {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, BlueprintAssignable)
	FOnUnHoverSkill OnUnHoverSkillDelegate {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn = "true"))
	FSkillChooseData Data {};

private:
	/* true when state > lockPrepare */
	bool m_isDisable { false };
};
