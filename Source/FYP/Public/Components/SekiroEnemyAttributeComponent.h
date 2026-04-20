#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "SekiroEnemyAttributeComponent.generated.h"

class USekiroCombatComponent;
class UAnimMontage;
class UUserWidget;

// ===== 「避」Perilous Attack Delegate =====
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPerilousAttackStarted);

UCLASS( ClassGroup=(Sekiro), meta=(BlueprintSpawnableComponent) )
class FYP_API USekiroEnemyAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USekiroEnemyAttributeComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Simple AI Logic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy")
	bool bAutoAttack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy")
	float AttackInterval = 3.0f;

	/** Number of attacks per combo cycle (e.g. 4 for Combo_Attack_04). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy", meta=(ClampMin="1"))
	int32 ComboAttackCount = 4;

	/** Delay in seconds between each Attack() call in a combo (should align with combo window, e.g. 0.45s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy", meta=(ClampMin="0.1"))
	float ComboAttackInterval = 0.45f;

	/** 是否令敵人一直面向玩家（打主角而唔係打空氣） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy")
	bool bFacePlayer = true;

	/** 面向玩家嘅有效距離（超過就唔轉向） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy", meta=(ClampMin="0"))
	float FacePlayerRange = 1500.f;

	/** 轉向玩家嘅速度（愈大愈快） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy", meta=(ClampMin="0.1"))
	float FacePlayerSpeed = 8.f;

	// ===== 「避」Perilous Attack 設定 =====

	/** Perilous 攻擊動畫（橫掃 Sweep / 突刺 Thrust） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy|Perilous")
	TArray<UAnimMontage*> PerilousAttackMontages;

	/** 每次 Combo 結束後觸發 Perilous 攻擊的機率 (0.0 = 0%, 1.0 = 100%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy|Perilous", meta=(ClampMin="0.0", ClampMax="1.0"))
	float PerilousAttackChance = 0.3f;

	/** Perilous 攻擊的傷害倍率（相對於普通攻擊） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy|Perilous", meta=(ClampMin="0.1"))
	float PerilousAttackDamageMultiplier = 2.0f;

	/** Perilous 攻擊開始時廣播（用於觸發「危」字 UI） */
	UPROPERTY(BlueprintAssignable, Category="Sekiro|Enemy|Perilous")
	FOnPerilousAttackStarted OnPerilousAttackStarted;

	/** 危攻擊命中時播放的音效（預設自動載入處決聲音） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy|Perilous")
	TObjectPtr<class USoundBase> PerilousAttackHitSound;

protected:
	float TimeSinceLastAttack = 0.0f;

	USekiroCombatComponent* CombatComp = nullptr;

	FTimerHandle ComboTimerHandle;
	int32 ComboAttacksRemaining = 0;

	/** 是否正在播 Perilous 攻擊動畫 */
	bool bIsPerilousAttacking = false;

	void StartComboAttackCycle();
	void OnComboAttackTimer();

	/** 嘗試觸發 Perilous 攻擊，成功返回 true */
	bool TryPerilousAttack();

	/** Perilous 攻擊動畫結束回調 */
	UFUNCTION()
	void OnPerilousAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// ===== 「避」字 Widget 直接顯示（繞過 HUD 依賴） =====
	UPROPERTY()
	TSubclassOf<UUserWidget> CachedPerilousWarningWidgetClass;

	UPROPERTY()
	TObjectPtr<class UUserWidget> PerilousWarningWidgetInstance;

	void ShowPerilousWarningWidget();

	/** 自身監聽自身 delegate — 任何人 Broadcast 都觸發 */
	UFUNCTION()
	void InternalOnPerilousAttackStarted();
};
