#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SekiroHUD.generated.h"

UCLASS()
class FYP_API ASekiroHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, Category="Sekiro|UI")
	TSubclassOf<class USekiroWidgetBase> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category="Sekiro|UI")
	TObjectPtr<class USekiroWidgetBase> HUDWidget;

protected:
	// 玩家事件回調
	UFUNCTION()
	void OnPlayerHealthChanged(float NewHealth, float MaxHealth);

	UFUNCTION()
	void OnPlayerPostureChanged(float CurrentPosture, float MaxPosture);

	// 敵人事件回調
	UFUNCTION()
	void OnEnemyHealthChanged(float NewHealth, float MaxHealth);

	UFUNCTION()
	void OnEnemyPostureChanged(float CurrentPosture, float MaxPosture);

	// ===== 「危」Perilous Attack Warning =====
	UFUNCTION()
	void OnPerilousAttackStarted();

	/** 是否正在顯示「危」字 */
	bool bShowPerilousWarning = false;

	/** 「危」字顯示剩餘時間 */
	float PerilousWarningTimer = 0.f;

	/** 「危」字顯示持續時間（秒） */
	float PerilousWarningDuration = 1.5f;
};

