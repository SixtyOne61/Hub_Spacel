// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Util/EnumUtil.h"
#include "TargetUserWidget.generated.h"


/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UTargetUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	enum class EState
	{
		StateNormal,
		StateHover,
		StateLock
	};

protected:
	void NativeConstruct() override;
	void NativeDestruct() override;

	void NativeTick(const FGeometry& _myGeometry, float _deltaTime) override;

private:
	bool hideAlly();

	UFUNCTION()
	void OnUnTargetPlayer(AActor* _target);

	UFUNCTION()
	void OnTargetPlayer(AActor* _target);

	/* callback method when state change */
	void onChangeStateNormal();
	void onChangeStateHover();
	void onChangeStateLock();

	/* set color of target image*/
	void setTargetImage(FLinearColor const& _color);

	/* event button */
	UFUNCTION()
	void OnPressed();

	UFUNCTION()
	void OnHovered();

public:
	UPROPERTY(VisibleAnywhere)
	class AActor* Owner { nullptr };

private:
	UPROPERTY()
	class UButton* TargetButton { nullptr };

	UPROPERTY()
	class UImage* TargetImage { nullptr };

	UPROPERTY(EditAnywhere)
	FLinearColor BaseColor {};

	UPROPERTY(EditAnywhere)
	FLinearColor HoverColor {};

	UPROPERTY(EditAnywhere)
	FLinearColor LockColor {};

	/* state of target */
	EnumUtil::EnumCallback<EState> m_state { };

	/* because unhovered call all frame */
	bool m_isHovered { false };
};

namespace EnumUtil
{
	template<>
	inline int count<UTargetUserWidget::EState>() { return (int)UTargetUserWidget::EState::StateLock; }
}