#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SekiroGameHUDWidget.generated.h"

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

	struct FFlashInfo
	{
		UWidget* Widget;
		float Timer;
	};
	TArray<FFlashInfo> ActiveFlashes;
};
