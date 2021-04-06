// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "SelectorSkillWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateSkill, ESkillType, _type, uint8, _level);

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USelectorSkillWidget : public UUserWidget
{
	GENERATED_BODY()

	friend class UPreparePhaseWidget;

public:
	USelectorSkillWidget(FObjectInitializer const& _objectInitializer);

protected:
	void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void SetTypeBackground(FSlateColor const& _color);
	
private:
	UFUNCTION()
	void OnPlus();

	UFUNCTION()
	void OnMinus();

	void onLevel(uint8 _delta);
	inline void setLock(bool _isLock) { m_isLock = _isLock; }
	void updateBorderColor();

public:
	UPROPERTY(EditAnywhere)
	class UTexture2D* TypeTexture { nullptr };

	UPROPERTY(EditAnywhere)
	TArray<class UTexture2D*> LevelTexture { nullptr, nullptr, nullptr };

	UPROPERTY(EditAnywhere)
	FLinearColor BackgroundColor {};

	UPROPERTY(EditAnywhere)
	FLinearColor LockColor {};

	UPROPERTY(EditAnywhere)
	ESkillType SkillType { ESkillType::None };

private:
	UPROPERTY()
	class UButton* PlusButton { nullptr };

	UPROPERTY()
	class UButton* MinusButton { nullptr };

	UPROPERTY()
	TArray<class UImage*> Images;

	UPROPERTY()
	TArray<class UBorder*> Borders;

	UPROPERTY()
	class UImage* Type;

	UPROPERTY()
	class UTextBlock* PointText { nullptr };

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnUpdateSkill OnUpdateSkillDelegate {};

	bool m_isLock {false};
	uint8 m_currentLevel {0};
};
