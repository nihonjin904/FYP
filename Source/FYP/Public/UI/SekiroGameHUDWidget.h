#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SekiroGameHUDWidget.generated.h"

class UImage;
class UCanvasPanel;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class ASekiroCharacter;

/**
 * 遊戲操作 HUD Widget 基類
 * 當玩家按下按鍵時，對應的圖示會閃爍
 * 用法：將 WBP_GameHUD 的父類改為此類別
 */
UCLASS()
class FYP_API USekiroGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 根據名稱讓指定 Widget 閃爍
	UFUNCTION(BlueprintCallable, Category = "Sekiro|UI")
	void FlashWidgetByName(const FName& WidgetName);

	// 靜態獲取實例（方便從任何地方調用）
	static USekiroGameHUDWidget* GetInstance() { return Instance; }

	// ===== Minimap BindWidget References =====
	UPROPERTY(meta=(BindWidgetOptional))
	UImage* MinimapImage = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* PlayerArrow = nullptr;

	// Canvas panel overlay (same position/size as MinimapImage) for checkpoint dots
	UPROPERTY(meta=(BindWidgetOptional))
	UCanvasPanel* MinimapDotsPanel = nullptr;

	// Material used to create the circular mask MID
	UPROPERTY(EditDefaultsOnly, Category="Minimap")
	UMaterialInterface* MinimapMaskMaterial = nullptr;

	// Minimap display size in pixels (should match MinimapImage size in WBP)
	UPROPERTY(EditDefaultsOnly, Category="Minimap")
	float MinimapDisplaySize = 300.f;

	// Minimap world capture radius (should match OrthoWidth in SekiroCharacter)
	UPROPERTY(EditDefaultsOnly, Category="Minimap")
	float MinimapOrthoRadius = 1500.f; // OrthoWidth/2

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 閃爍持續時間（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|UI")
	float FlashDuration = 0.15f;

	// 閃爍時的最低不透明度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|UI")
	float FlashMinOpacity = 0.2f;

private:
	static USekiroGameHUDWidget* Instance;

	// Dynamic material instance for minimap circle mask
	UMaterialInstanceDynamic* MinimapMID = nullptr;

	// Checkpoint dot data
	struct FMinimapDotInfo
	{
		FString ActorName;   // Checkpoint actor name (used to check activation)
		FVector WorldPos;    // World position of checkpoint
		UImage* DotWidget = nullptr; // UImage dot in MinimapDotsPanel
	};
	TArray<FMinimapDotInfo> CheckpointDots;

	// Refresh dots position+color every N seconds (not every tick)
	float MinimapDotTimer = 0.f;
	static constexpr float MinimapDotRefreshInterval = 0.5f;

	void BuildCheckpointDots(); // Called once in NativeConstruct
	void RefreshCheckpointDots(ASekiroCharacter* Player); // Called periodically

	struct FFlashInfo
	{
		UWidget* Widget;
		float Timer;
	};
	TArray<FFlashInfo> ActiveFlashes;
};
