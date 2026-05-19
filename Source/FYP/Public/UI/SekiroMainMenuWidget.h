#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SekiroGameInstance.h"
#include "Widgets/Input/SSlider.h"
#include "SekiroMainMenuWidget.generated.h"

/**
 * 全 C++ 主菜單：Title + Start + 難度選擇 + 背景圖
 * 不需要 Widget Blueprint — 直接用 Slate 構建 UI
 */
UCLASS()
class FYP_API USekiroMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// ===== 按鈕回調 =====
	FReply OnStartClicked();
	FReply OnRestartClicked();
	FReply OnEasyClicked();
	FReply OnNormalClicked();
	FReply OnHardClicked();

	// ===== 難度狀態 =====
	ESekirodifficulty SelectedDifficulty = ESekirodifficulty::Normal;

	// ===== 視覺更新 =====
	TSharedPtr<SBorder> EasyBorder;
	TSharedPtr<SBorder> NormalBorder;
	TSharedPtr<SBorder> HardBorder;
	TSharedPtr<STextBlock> DifficultyLabel;

	void UpdateDifficultyVisuals();

	// ===== 背景圖 =====
	TSharedPtr<FSlateDynamicImageBrush> BackgroundBrush;

	// ===== BGM Volume Slider =====
	TSharedPtr<SSlider> BGMVolumeSlider;
	void OnBGMVolumeChanged(float NewValue);

	// ===== SFX Volume Slider =====
	TSharedPtr<SSlider> SFXVolumeSlider;
	void OnSFXVolumeChanged(float NewValue);

	// ===== 工具函數 =====
	TSharedRef<SWidget> MakeDifficultyButton(
		const FString& Label,
		const FString& Desc,
		TSharedPtr<SBorder>& OutBorder,
		FOnClicked ClickDelegate);

	TSharedRef<SWidget> MakeMenuButton(
		const FString& Label,
		float FontSize,
		FLinearColor BgColor,
		FOnClicked ClickDelegate);
};
