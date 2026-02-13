#pragma once

#include "Camera/CameraShakeBase.h"
#include "Components/SceneComponent.h"
#include "Components/SekiroDeflectComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SekiroCharacter.generated.h"

class USekiroPostureComponent;
class USekiroDeflectComponent;
class USekiroCombatComponent;
class USekiroAttributeComponent;
class UWidgetComponent;
class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class FYP_API ASekiroCharacter : public ACharacter {
  GENERATED_BODY()

public:
  ASekiroCharacter();

protected:
  virtual void BeginPlay() override;

public:
  virtual void Tick(float DeltaTime) override;
  virtual void SetupPlayerInputComponent(
      class UInputComponent *PlayerInputComponent) override;

  // Components
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|Components")
  TObjectPtr<USekiroPostureComponent> PostureComponent;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|Components")
  TObjectPtr<USekiroDeflectComponent> DeflectComponent;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|Components")
  TObjectPtr<USekiroCombatComponent> CombatComponent;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|Components")
  TObjectPtr<USekiroAttributeComponent> AttributeComponent;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
  TObjectPtr<UWidgetComponent> OverheadWidget;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
  TObjectPtr<UWidgetComponent> DeathblowWidget;

  UFUNCTION()
  void OnPostureBroken();

  UFUNCTION()
  void HandleParryResult(EParryResult Result);

  UFUNCTION()
  void OnDeath();

  UFUNCTION(BlueprintCallable, Category = "Sekiro|Combat")
  void Attack();

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
  TObjectPtr<USpringArmComponent> CameraBoom;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
  TObjectPtr<UCameraComponent> FollowCamera;

  // Procedural Weapon
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|Weapon")
  TObjectPtr<UStaticMeshComponent> WeaponMesh;

  /** 擋刀時用嘅旋轉軸：手 (hand_r) → 此 Pivot → WeaponMesh。擋刀時只轉呢個
   * Pivot，刀就會打橫，唔會被動畫蓋過。 */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|Weapon")
  TObjectPtr<USceneComponent> BlockWeaponPivot;

  /** 擋刀時套用到武器上的額外旋轉（打橫放等）。留空則用
   * WeaponMesh。已棄用：建議用 BlockWeaponPivot。 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|Weapon",
            meta = (DisplayName = "Block Weapon Component (Optional Override)"))
  TObjectPtr<USceneComponent> BlockWeaponComponent;

  /** 擋刀時武器相對手嘅旋轉（例如打橫：(0, 0, 90)）。非擋刀時會還原為 0。 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|Weapon",
            meta = (DisplayName = "Block Weapon Rotation"))
  FRotator BlockWeaponRotationWhenBlocking = FRotator(0.f, 0.f, 90.f);

  // Animations
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
  TObjectPtr<UAnimMontage> AttackMontage;

  /** 開始擋刀 (BlockStart_Root) - 按 Block 時播一次 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
  TObjectPtr<UAnimMontage> ParryAttemptMontage;

  /** 持續擋刀 (BlockLoop_Root) - 按住 Block 時循環，需在 Montage 裡把 Section
   * 設為 Loop */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
  TObjectPtr<UAnimMontage> BlockLoopMontage;

  /** 擋刀被擊中 (BlockHit_Root) - 格擋成功時播一次 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
  TObjectPtr<UAnimMontage> BlockHitMontage;

  /** 結束擋刀 (BlockEnd_Root) - 放開 Block 時播一次 */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
  TObjectPtr<UAnimMontage> BlockEndMontage;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
  TObjectPtr<UAnimMontage> ParrySuccessMontage; // Perfect Parry spark

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
  TObjectPtr<UAnimMontage> HitMontage; // Failed parry (Take damage)

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
  TObjectPtr<UAnimMontage> ExecutionMontage;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
  TObjectPtr<UAnimMontage> StunMontage;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
  TObjectPtr<UAnimMontage> DeathMontage;

  // ========== 精準格擋 (Perfect Parry) Feedback ==========

  /** 精準格擋時播放嘅音效（清脆金屬碰撞聲） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Sekiro|Feedback|Perfect Parry")
  TObjectPtr<class USoundBase> PerfectParrySound;

  /** 精準格擋時嘅 Niagara 火花特效（明亮橙黃色） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Sekiro|Feedback|Perfect Parry")
  TObjectPtr<class UNiagaraSystem> PerfectParryNiagara;

  /** 精準格擋時嘅 Camera Shake（鏡頭震動） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Sekiro|Feedback|Perfect Parry")
  TSubclassOf<UCameraShakeBase> PerfectParryCameraShake;

  /** 精準格擋 Hit Stop 時長（秒），0 = 唔開 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Sekiro|Feedback|Perfect Parry",
            meta = (ClampMin = "0", ClampMax = "0.5"))
  float PerfectParryHitStopDuration = 0.05f;

  /** 精準格擋 Hit Stop 時嘅時間縮放 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Sekiro|Feedback|Perfect Parry",
            meta = (ClampMin = "0.01", ClampMax = "1"))
  float PerfectParryHitStopTimeScale = 0.08f;

  // ========== 普通格擋 (Blocked) Feedback ==========

  /** 普通格擋時播放嘅音效（較鈍嘅金屬碰撞聲） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Sekiro|Feedback|Block")
  TObjectPtr<class USoundBase> BlockSound;

  /** 普通格擋時嘅 Niagara 火花特效（較細較暗） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Sekiro|Feedback|Block")
  TObjectPtr<class UNiagaraSystem> BlockNiagara;

  /** 普通格擋時嘅 Camera Shake（鏡頭震動，比 Perfect 弱） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Sekiro|Feedback|Block")
  TSubclassOf<UCameraShakeBase> BlockCameraShake;

  /** 普通格擋 Hit Stop 時長（秒） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Sekiro|Feedback|Block",
            meta = (ClampMin = "0", ClampMax = "0.5"))
  float BlockHitStopDuration = 0.03f;

  /** 普通格擋 Hit Stop 時嘅時間縮放 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "Sekiro|Feedback|Block",
            meta = (ClampMin = "0.01", ClampMax = "1"))
  float BlockHitStopTimeScale = 0.1f;

  // ========== 共用 / 攻擊方 Feedback ==========

  /** 攻擊方（敵人）嗰邊都生成嘅火花 Niagara（喺攻擊者嘅武器位生成） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|Feedback")
  TObjectPtr<class UNiagaraSystem> AttackerSparkNiagara;

  /** 舊版兼容：擋刀時嘅 Cascade 粒子（可留空） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|Feedback")
  TObjectPtr<class UParticleSystem> ParryBlockParticle;

  /** 擋/彈刀時呼叫，Blueprint 可覆寫做 Screen Flash、額外 VFX */
  UFUNCTION(BlueprintNativeEvent, Category = "Sekiro|Feedback")
  void OnParryBlockFeedback();
  virtual void OnParryBlockFeedback_Implementation();

  // State
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|State")
  bool bIsBlocking = false;

  /** 是否鎖定敵人（鎖定時鏡頭／面向跟住目標） */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|State")
  bool bIsLockedOn = false;

  /** 當前鎖定嘅目標（可處決／可攻擊嘅敵人） */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|State")
  TObjectPtr<AActor> LockedTarget;

  /** 非玩家角色（敵人）是否自動面向玩家 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|AI")
  bool bFacePlayerAsAI = true;

  /** 處決成功時播 Execution Montage（由 OnExecutionTriggered 觸發） */
  UFUNCTION()
  void OnExecutionTriggered(AActor *Target);

  UFUNCTION()
  void OnAttackStartedForTrail();
  UFUNCTION()
  void OnAttackEndedForTrail();

  /** 攻擊開始/結束時呼叫，Blueprint 可覆寫做刀光軌跡 */
  UFUNCTION(BlueprintNativeEvent, Category = "Sekiro|Feedback")
  void K2_OnAttackStarted();
  virtual void K2_OnAttackStarted_Implementation();
  UFUNCTION(BlueprintNativeEvent, Category = "Sekiro|Feedback")
  void K2_OnAttackEnded();
  virtual void K2_OnAttackEnded_Implementation();

  // Input
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
  TObjectPtr<UInputMappingContext> DefaultMappingContext;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
  TObjectPtr<UInputAction> JumpAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
  TObjectPtr<UInputAction> MoveAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
  TObjectPtr<UInputAction> LookAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
  TObjectPtr<UInputAction> BlockAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
  TObjectPtr<UInputAction> AttackAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
  TObjectPtr<UInputAction> ExecutionAction;

  /** 鎖定／解除鎖定（例如 Q 或 R3） */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
  TObjectPtr<UInputAction> LockOnAction;

  /** 鎖定搜尋半徑（公尺） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|LockOn",
            meta = (ClampMin = "500", ClampMax = "5000"))
  float LockOnRange = 1500.f;

  /** 鎖定時鏡頭轉向目標嘅速度 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|LockOn",
            meta = (ClampMin = "1", ClampMax = "20"))
  float LockOnRotationSpeed = 8.f;

  /** 鎖定時瞄準目標嘅高度偏移（避免鎖地面；數值愈大鏡頭愈抬高） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|LockOn",
            meta = (ClampMin = "0", ClampMax = "300"))
  float LockOnTargetZOffset = 140.f;

  /** 鎖定時將 Camera Boom 的 Socket Offset Z 抬高（令相機位置唔會太低） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|LockOn",
            meta = (ClampMin = "-200", ClampMax = "200"))
  float LockOnCameraSocketOffsetZ = 60.f;

  /** 鎖定時 Pitch 夾角限制（防止鏡頭向下） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|LockOn",
            meta = (ClampMin = "-89", ClampMax = "0"))
  float LockOnPitchMin = -25.f;

  /** 鎖定時 Pitch 夾角限制（防止鏡頭向上過多） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|LockOn",
            meta = (ClampMin = "0", ClampMax = "89"))
  float LockOnPitchMax = 25.f;

  /** 鎖定時使用固定俯視 Pitch（唔會因敵人距離而望天/望地） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|LockOn")
  bool bLockOnUseFixedPitch = true;

  /** 固定俯視 Pitch（負數 = 俯視向下；建議 -25 ~ -45） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|LockOn",
            meta = (ClampMin = "-89", ClampMax = "0"))
  float LockOnFixedPitch = -35.f;

  /** 鎖定目標嘅 Actor Tag（例如 "Enemy"；BP_SekiroEnemy 需加此 Tag） */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|LockOn")
  FName LockOnTargetTag = FName("Enemy");

  /** 按一下切換鎖定；若無目標則解除 */
  UFUNCTION(BlueprintCallable, Category = "Sekiro|Combat")
  void ToggleLockOn();

  /** BlockStart 播完後若仍按住 Block，接 BlockLoop */
  UFUNCTION()
  void OnBlockStartMontageEnded(UAnimMontage *Montage, bool bInterrupted);

  /** BlockHit 播完後若仍按住 Block，接回 BlockLoop */
  UFUNCTION()
  void OnBlockHitMontageEnded(UAnimMontage *Montage, bool bInterrupted);

protected:
  void Move(const FInputActionValue &Value);
  void Look(const FInputActionValue &Value);

  void StartBlock();
  void StopBlock();
  void Execution(const FInputActionValue &Value);
  void LockOnPressed();

  /** 用於解除鎖定時清除上一個目標嘅 Outline（Custom Depth） */
  TObjectPtr<AActor> PreviousLockedTarget;

  /** 用於防止 BlockAction 立即 Completed 造成一按即放（保護時間） */
  double LastBlockInputTimeSeconds = -1000.0;

  /** 緩存鎖定前 CameraBoom SocketOffset，用於解鎖還原 */
  FVector DefaultCameraBoomSocketOffset = FVector::ZeroVector;
};
