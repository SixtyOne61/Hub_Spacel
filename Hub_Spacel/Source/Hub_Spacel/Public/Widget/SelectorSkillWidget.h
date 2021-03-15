// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "SelectorSkillWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClickLevel, ESkillType, _type, uint8, _value);

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
	virtual void NativeConstruct() override;
	
private:
	UFUNCTION()
	void OnLevel1();

	UFUNCTION()
	void OnLevel2();

	UFUNCTION()
	void OnLevel3();

	void onLevel(uint8 _level);
	void reset();
	inline void setLock(bool _isLock) { m_isLock = _isLock; }

public:
	UPROPERTY(EditAnywhere)
	ESkillType SkillType { ESkillType::None };

	UPROPERTY(EditAnywhere)
	FSlateColor OffColor {};

	UPROPERTY(EditAnywhere)
	FSlateColor OnColor {};

private:
	UPROPERTY()
	TArray<class UButton*> Buttons;

	UPROPERTY()
	TArray<class UImage*> Images;

	UPROPERTY()
	FOnClickLevel OnClickLevelDelegate {};

	bool m_isLock {false};
};
