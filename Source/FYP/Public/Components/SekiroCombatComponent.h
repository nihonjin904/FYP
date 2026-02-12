#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SekiroCombatComponent.generated.h"

class USekiroPostureComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackPerformed, FGameplayTag,
                                            AttackType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExecutionTriggered, AActor *,
                                            Target);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnded);

UCLASS(ClassGroup = (Sekiro), meta = (BlueprintSpawnableComponent))
class FYP_API USekiroCombatComponent : public UActorComponent {
  GENERATED_BODY()

public:
  USekiroCombatComponent();

protected:
  virtual void BeginPlay() override;

public:
  // Attempt to perform an attack
  UFUNCTION(BlueprintCallable, Category = "Sekiro|Combat")
  void RequestAttack();

  /** 嘗試處決（有鎖定或前方可處決目標則執行）。返回 true 表示已處決。 */
  UFUNCTION(BlueprintCallable, Category = "Sekiro|Combat")
  bool RequestExecution();

  // 執行攻擊判定（由 Animation Notify 調用）
  UFUNCTION(BlueprintCallable, Category = "Sekiro|Combat")
  void PerformAttackHitCheck();

  // 重置攻擊命中標記（由 AnimNotifyState 在攻擊窗口開始時調用）
  UFUNCTION(BlueprintCallable, Category = "Sekiro|Combat")
  void ResetAttackHit();

  // Attempt to perform an execution (Deathblow)
  UFUNCTION(BlueprintCallable, Category = "Sekiro|Combat")
  bool TryExecuteTarget(AActor *TargetActor);

  // Check if a target is executable
  UFUNCTION(BlueprintCallable, Category = "Sekiro|Combat")
  bool CanExecuteTarget(AActor *TargetActor) const;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|Combat")
  float ExecutionRange = 150.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|Combat")
  float AttackRange = 200.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|Combat")
  float AttackPostureDamage = 20.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|Combat")
  FGameplayTag ContainerTag_Stunned;

  UPROPERTY(BlueprintAssignable, Category = "Sekiro|Combat")
  FOnAttackPerformed OnAttackPerformed;

  UPROPERTY(BlueprintAssignable, Category = "Sekiro|Combat")
  FOnExecutionTriggered OnExecutionTriggered;

  /** 開始攻擊 Combo 時（刀光軌跡可在 Blueprint 聽呢個開） */
  UPROPERTY(BlueprintAssignable, Category = "Sekiro|Combat")
  FOnAttackStarted OnAttackStarted;

  /** 攻擊 Combo 完全結束時（刀光軌跡可在 Blueprint 聽呢個關） */
  UPROPERTY(BlueprintAssignable, Category = "Sekiro|Combat")
  FOnAttackEnded OnAttackEnded;

  // ========== COMBO 系統 ==========

  // 存放 Combo 動畫 Montage（按順序）
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|Combo")
  TArray<UAnimMontage *> ComboMontages;

  // 當前 Combo 索引（0 = 第一段攻擊）
  UPROPERTY(BlueprintReadOnly, Category = "Sekiro|Combo")
  int32 ComboIndex = 0;

  // 是否在可以接下一段攻擊的窗口內
  UPROPERTY(BlueprintReadOnly, Category = "Sekiro|Combo")
  bool bCanCombo = false;

  // 是否正在攻擊中
  UPROPERTY(BlueprintReadOnly, Category = "Sekiro|Combo")
  bool bIsAttacking = false;

  // 輸入緩衝：玩家是否已經按了下一次攻擊（等待 Combo 窗口開啟）
  UPROPERTY(BlueprintReadOnly, Category = "Sekiro|Combo")
  bool bComboQueued = false;

  // Combo 窗口時間（秒）- 在動畫某個時間點後可以按下一段
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|Combo")
  float ComboWindowTime = 0.3f;

  // 上一次執行 Combo 動作的時間（用於判斷中斷是否由自身觸發）
  double LastComboActionTime = 0.0;

  // Timer Handle for Combo Window
  FTimerHandle ComboWindowHandle;

  // 重置 Combo 到初始狀態
  UFUNCTION(BlueprintCallable, Category = "Sekiro|Combo")
  void ResetCombo();

  // 開啟 Combo 窗口（由 Anim Notify 調用）
  UFUNCTION(BlueprintCallable, Category = "Sekiro|Combo")
  void EnableComboWindow();

  // 關閉 Combo 窗口（由 Anim Notify 調用）
  UFUNCTION(BlueprintCallable, Category = "Sekiro|Combo")
  void DisableComboWindow();

  // 當 Montage 結束時的回調
  UFUNCTION()
  void OnMontageEnded(UAnimMontage *Montage, bool bInterrupted);

protected:
  // Helper to get posture component from an actor
  USekiroPostureComponent *GetTargetPosture(AActor *Target) const;

  // 獲取角色的動畫實例
  class UAnimInstance *GetOwnerAnimInstance() const;

private:
  // 標記本次攻擊窗口是否已經命中過（防止同一次攻擊多次傷害）
  bool bHasHit = false;
};
