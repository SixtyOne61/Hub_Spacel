// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "SkillItemWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChooseSkill);

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USkillItemWidget : public UUserWidget
{
	GENERATED_BODY()

	struct FData
	{
		ESkill m_id;
		FSlateColor m_backgroundColor{};
		FString m_title{};
		FString m_desc{};
		class UTexture2D* m_icon { nullptr };
		ESkillType m_type;

		FData& operator=(FData&& _move)
		{
			m_id = std::move(_move.m_id);
			m_backgroundColor = std::move(_move.m_backgroundColor);
			m_title = std::move(_move.m_title);
			m_desc = std::move(_move.m_desc);
			m_icon = std::move(_move.m_icon);
			m_type = std::move(_move.m_type);

			_move.m_id = {};
			_move.m_backgroundColor = {};
			_move.m_title = {};
			_move.m_desc = {};
			_move.m_icon = {};
			_move.m_type = {};
			return *this;
		}

	} m_data;

public:
	void setupItems(FData && _data);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Event")
	void BP_Setup(FSlateColor const& _backgroundColor, FString const& _title, FString const& _desc, class UTexture2D* _icon);

	UFUNCTION(BlueprintCallable, Category = "UI|Event")
	void OnChooseSkill();

	UFUNCTION(BlueprintCallable, Category = "UI|Event")
	void OnHover();

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, BlueprintAssignable)
	FOnChooseSkill OnChooseSkillDelegate {};
};
