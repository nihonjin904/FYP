#include "Components/SekiroCombatComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNotifyState_AttackWindow.h"
#include "Characters/SekiroCharacter.h"
#include "Components/SekiroAttributeComponent.h"
#include "Components/SekiroDeflectComponent.h"
#include "Components/SekiroPostureComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MotionWarpingComponent.h"
#include "TimerManager.h"

USekiroCombatComponent::USekiroCombatComponent() {
  PrimaryComponentTick.bCanEverTick = false;

  // Default tag for stunned state
  ContainerTag_Stunned =
      FGameplayTag::RequestGameplayTag(FName("State.Stunned"));
}

void USekiroCombatComponent::BeginPlay() { Super::BeginPlay(); }

void USekiroCombatComponent::RequestAttack() {
  AActor *Owner = GetOwner();
  if (!Owner)
    return;

  // ========== COMBO 系統邏輯 ==========

  // 如果有 Combo Montages，使用 Combo 系統
  if (ComboMontages.Num() > 0) {
    UAnimInstance *AnimInstance = GetOwnerAnimInstance();
    if (!AnimInstance)
      return;

    // 情況 1：沒有在攻擊中 - 開始第一段攻擊
    bool bIsValidCombo0 = (ComboMontages.IsValidIndex(0) && ComboMontages[0] != nullptr);
    SpecialAttackChance = 1.0f; // 強制 100% 發動機率，供驗證大招動畫
    
    if (GEngine) {
        GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow, FString::Printf(TEXT("RequestAttack Pointers: IsAtk=%d, Cmb0_Valid=%d, SpclM=%d, SpclChance=%.2f"), bIsAttacking, bIsValidCombo0, SpecialMontages.Num(), SpecialAttackChance));
    }

    if (!bIsAttacking) {
      
      // --- BOSS RNG OVERRIDE (30%預設機率觸發獨立大招) ---
      if (SpecialMontages.Num() > 0 && FMath::FRand() <= SpecialAttackChance) {
        bIsAttacking = true;
        ComboIndex = 0; // 重置一般連擊計數器
        bCanCombo = false;
        bComboQueued = false;

        LastComboActionTime = GetWorld()->GetTimeSeconds();
        OnAttackStarted.Broadcast();

        int32 RandomIndex = FMath::RandRange(0, SpecialMontages.Num() - 1);
        float Duration = AnimInstance->Montage_Play(SpecialMontages[RandomIndex]);

        if (Duration <= 0.0f) {
          bIsAttacking = false;
          return;
        }

        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &USekiroCombatComponent::OnMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, SpecialMontages[RandomIndex]);
        
        return; // 直接搶斷，終止下方 ComboMontages 執行
      }
      // --- END BOSS RNG OVERRIDE ---

      if (ComboMontages.IsValidIndex(0) && ComboMontages[0]) {
        bIsAttacking = true;
        ComboIndex = 0;
        bCanCombo = false;
        bComboQueued = false;

        LastComboActionTime = GetWorld()->GetTimeSeconds();
        OnAttackStarted.Broadcast();
        float Duration = AnimInstance->Montage_Play(ComboMontages[ComboIndex]);

        if (Duration <= 0.0f) {
          bIsAttacking = false;
          return;
        }

        // 自動檢測 Montage 是否有 AnimNotifyState_AttackWindow
        // 若沒有，宇用 timer fallback 自動誈算攻擊判定窗口
        auto MontageHasAttackWindow = [](UAnimMontage* M) -> bool {
          if (!M) return false;
          for (const FAnimNotifyEvent& N : M->Notifies) {
            if (N.NotifyStateClass &&
                N.NotifyStateClass->IsA(UAnimNotifyState_AttackWindow::StaticClass()))
              return true;
          }
          return false;
        };

        if (!MontageHasAttackWindow(ComboMontages[ComboIndex])) {
          StartAutoAttackWindow(Duration); // fallback
        }

        // 綁定 Montage 結束回調
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &USekiroCombatComponent::OnMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, ComboMontages[ComboIndex]);

        float WindowOpenTime = Duration > 0.0f ? Duration * 0.5f : ComboWindowTime;
        GetWorld()->GetTimerManager().SetTimer(
            ComboWindowHandle,
            [this]() { if (bIsAttacking) EnableComboWindow(); },
            WindowOpenTime, false);
      }
    }
    // 情況 2：正在攻擊中，且在 Combo 窗口內 - 接下一段
    else if (bCanCombo) {
      int32 NextIndex = ComboIndex + 1;

      if (ComboMontages.IsValidIndex(NextIndex) && ComboMontages[NextIndex]) {
        ComboIndex = NextIndex;
        bCanCombo = false;
        bComboQueued = false;

        LastComboActionTime = GetWorld()->GetTimeSeconds();
        float Duration = AnimInstance->Montage_Play(ComboMontages[ComboIndex]);

        if (Duration <= 0.0f)
          return;

        // 自動檢測：若無 AnimNotifyState_AttackWindow，使用 timer fallback
        bool bHasWindow = false;
        for (const FAnimNotifyEvent& N : ComboMontages[ComboIndex]->Notifies)
          if (N.NotifyStateClass &&
              N.NotifyStateClass->IsA(UAnimNotifyState_AttackWindow::StaticClass()))
          { bHasWindow = true; break; }
        if (!bHasWindow)
          StartAutoAttackWindow(Duration);

        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &USekiroCombatComponent::OnMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, ComboMontages[ComboIndex]);

        float WindowOpenTime = Duration > 0.0f ? Duration * 0.5f : ComboWindowTime;
        GetWorld()->GetTimerManager().SetTimer(
            ComboWindowHandle,
            [this]() { if (bIsAttacking) EnableComboWindow(); },
            WindowOpenTime, false);
      }
    }
    // 情況 3：正在攻擊中，但不在 Combo 窗口內 - 記住輸入，等待窗口開啟
    else {
      // 輸入緩衝：記住玩家想接招
      bComboQueued = true;
      if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Orange,
                                         TEXT("Input Queued!"));
    }
  }

  // === 攻擊判定邏輯已移除 ===
  // 攻擊判定現在由 AnimNotifyState_AttackWindow 在動畫中觸發
  // 這確保一次攻擊動畫只會造成一次傷害，無論玩家按多少次攻擊鍵
}


bool USekiroCombatComponent::RequestExecution() {
  AActor *Owner = GetOwner();
  if (!Owner)
    return false;

  // 若已鎖定敵人且該敵人可處決，直接對鎖定目標處決（架勢條滿即可）
  ASekiroCharacter* SekiroOwner = Cast<ASekiroCharacter>(Owner);
  if (SekiroOwner && SekiroOwner->LockedTarget && CanExecuteTarget(SekiroOwner->LockedTarget)) {
    if (TryExecuteTarget(SekiroOwner->LockedTarget))
      return true;
  }

  // Sphere Trace to find execution target
  FVector Start = Owner->GetActorLocation();
  FVector End = Start + (Owner->GetActorForwardVector() * ExecutionRange);

  FHitResult HitResult;
  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(Owner);

  bool bHit = GetWorld()->SweepSingleByChannel(
      HitResult, Start, End, FQuat::Identity, ECC_Pawn,
      FCollisionShape::MakeSphere(50.0f), QueryParams);

  if (bHit && HitResult.GetActor() && CanExecuteTarget(HitResult.GetActor()))
    return TryExecuteTarget(HitResult.GetActor());

  return false;
}

bool USekiroCombatComponent::TryExecuteTarget(AActor *TargetActor) {
  if (!CanExecuteTarget(TargetActor)) {
    return false;
  }

  // Trigger Execution
  OnExecutionTriggered.Broadcast(TargetActor);

  // 1. Play Execution Montage on Self (with Motion Warping to Target)
  // 2. Play Death Montage on Target
  // 3. Reset Target Posture or Kill Target

  USekiroAttributeComponent *TargetAttribute =
      TargetActor->FindComponentByClass<USekiroAttributeComponent>();
  if (TargetAttribute) {
    // Critical Hit: 10x Damage (or Instant Kill)
    TargetAttribute->ApplyDamage(100.0f * 10.0f);
  }

  if (GEngine)
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
                                     TEXT("DEATHBLOW EXECUTED!"));

  // Example Motion Warping setup (Conceptual):
  // UMotionWarpingComponent* MotionWarping =
  // GetOwner()->FindComponentByClass<UMotionWarpingComponent>(); if
  // (MotionWarping)
  // {
  //     MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation("ExecutionTarget",
  //     TargetActor->GetActorLocation(), TargetActor->GetActorRotation());
  // }

  return true;
}

bool USekiroCombatComponent::CanExecuteTarget(AActor *TargetActor) const {
  if (!TargetActor)
    return false;

  // Check distance
  float Distance = GetOwner()->GetDistanceTo(TargetActor);
  if (Distance > ExecutionRange)
    return false;

  // Check if target is stunned
  // Assuming target uses GameplayTags or has a specific component state
  // For this demo, we check if they have the "State.Stunned" tag
  if (!TargetActor->ActorHasTag(ContainerTag_Stunned.GetTagName())) {
    // Alternatively, check Posture Component directly
    USekiroPostureComponent *TargetPosture = GetTargetPosture(TargetActor);
    if (TargetPosture) {
      // If posture is broken (>= Max), they are vulnerable
      if (TargetPosture->CurrentPosture < TargetPosture->MaxPosture) {
        return false;
      }
    } else {
      return false;
    }
  }

  return true;
}

USekiroPostureComponent *
USekiroCombatComponent::GetTargetPosture(AActor *Target) const {
  if (!Target)
    return nullptr;
  return Target->FindComponentByClass<USekiroPostureComponent>();
}

// ========== COMBO 系統函數實現 ==========

void USekiroCombatComponent::ResetCombo() {
  if (bIsAttacking)
    OnAttackEnded.Broadcast();
  ComboIndex = 0;
  bCanCombo = false;
  bIsAttacking = false;
  bComboQueued = false;

  // Clear any pending timers
  if (GetWorld()) {
    GetWorld()->GetTimerManager().ClearTimer(ComboWindowHandle);
  }
  StopAutoAttackWindow();

  if (GEngine)
    GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Cyan,
                                     TEXT("Combo Reset"));
}

void USekiroCombatComponent::EnableComboWindow() {
  bCanCombo = true;
  if (GEngine)
    GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Green,
                                     TEXT("Combo Window Open!"));

  // 如果窗口開啟時已經有緩衝的輸入，立即執行下一段
  if (bComboQueued) {
    bComboQueued = false;
    RequestAttack();
  }
}

void USekiroCombatComponent::DisableComboWindow() { bCanCombo = false; }

void USekiroCombatComponent::OnMontageEnded(UAnimMontage *Montage,
                                            bool bInterrupted) {
  if (bInterrupted) {
    // 如果被中斷的是"舊"的 Montage（因為我們已切換到下一段 Combo），忽略它
    // 例如：Attack 2 打斷 Attack 1 時，OnMontageEnded(Attack1, interrupted=true) 觸發，
    // 但 ComboIndex 已經遞增到 1，所以 ComboMontages[ComboIndex] != Attack1 → 忽略
    if (ComboMontages.IsValidIndex(ComboIndex) &&
        ComboMontages[ComboIndex] != Montage) {
      return; // 故意的連招切換，忽略此中斷
    }
    // 真正的外部中斷（受傷等），重置 Combo
    ResetCombo();
    return;
  }

  // 正常結束：檢查是否有緩衝輸入
  if (bComboQueued) {
    bComboQueued = false;
    bCanCombo = true;
    RequestAttack();
  } else {
    bIsAttacking = false;
    bCanCombo = false;
    OnAttackEnded.Broadcast();
    ResetCombo();
  }
}



UAnimInstance *USekiroCombatComponent::GetOwnerAnimInstance() const {
  AActor *Owner = GetOwner();
  if (!Owner)
    return nullptr;

  ACharacter *Character = Cast<ACharacter>(Owner);
  if (!Character)
    return nullptr;

  USkeletalMeshComponent *Mesh = Character->GetMesh();
  if (!Mesh)
    return nullptr;

  return Mesh->GetAnimInstance();
}

// ========== 攻擊判定系統 ==========

void USekiroCombatComponent::ResetAttackHit() {
  bHasHit = false;
  
  if (GEngine)
    GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Cyan,
                                     TEXT("Attack Window Started - Reset Hit Flag"));
}

void USekiroCombatComponent::PerformAttackHitCheck() {
  // 如果本次攻擊窗口已經命中過，不再執行判定
  if (bHasHit)
    return;

  AActor *Owner = GetOwner();
  if (!Owner)
    return;

  // 廣播攻擊事件
  FGameplayTag AttackTag =
      FGameplayTag::RequestGameplayTag(FName("Action.Attack.Light"));
  OnAttackPerformed.Broadcast(AttackTag);

  // Hit Detection
  FVector Start = Owner->GetActorLocation();
  FVector End = Start + (Owner->GetActorForwardVector() * AttackRange);

  FHitResult HitResult;
  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(Owner);

  bool bHit = GetWorld()->SweepSingleByChannel(
      HitResult, Start, End, FQuat::Identity, ECC_Pawn,
      FCollisionShape::MakeSphere(50.0f), QueryParams);

  if (bHit && HitResult.GetActor()) {
    AActor *HitActor = HitResult.GetActor();

    // 標記本次攻擊窗口已經命中，防止多次傷害
    bHasHit = true;

    // Check for Components
    USekiroDeflectComponent *DeflectComp =
        HitActor->FindComponentByClass<USekiroDeflectComponent>();
    USekiroPostureComponent *PostureComp =
        HitActor->FindComponentByClass<USekiroPostureComponent>();
    USekiroAttributeComponent *AttributeComp =
        HitActor->FindComponentByClass<USekiroAttributeComponent>();

    float DamageToApply = 10.0f; // Base Health Damage

    if (DeflectComp) {
      // Try to Parry
      EParryResult Result = DeflectComp->TryParry(AttackTag);

      if (Result == EParryResult::Perfect) {
        if (GEngine)
          GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange,
                                           TEXT("PERFECT PARRY!"));
        // No Health Damage, No Posture Damage to Defender

        // Apply massive posture damage to Attacker (Self)
        USekiroPostureComponent *MyPosture =
            Owner->FindComponentByClass<USekiroPostureComponent>();
        if (MyPosture) {
          // Penalty: 3x normal posture damage
          MyPosture->AddPostureDamage(AttackPostureDamage * 3.0f);
        }
      } else if (Result == EParryResult::Blocked) {
        if (GEngine)
          GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue,
                                           TEXT("Blocked!"));
        // Blocked: No Health Damage, but Posture Damage
        if (PostureComp)
          PostureComp->AddPostureDamage(AttackPostureDamage * 0.5f);
      } else // Failed
      {
        if (GEngine)
          GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Hit!"));
        // Hit: Health Damage + Pause Posture Regen (0 damage)
        if (AttributeComp)
          AttributeComp->ApplyDamage(DamageToApply);
        if (PostureComp)
          PostureComp->AddPostureDamage(0.0f);
      }
    } else {
      // No deflect component, just take damage
      if (AttributeComp)
        AttributeComp->ApplyDamage(DamageToApply);
      if (PostureComp)
        PostureComp->AddPostureDamage(0.0f);
    }
  }

  // Draw Debug Line
  DrawDebugSphere(GetWorld(), End, 50.0f, 12, FColor::Red, false, 1.0f);
}

// ========== 自動攻擊窗口實現 ==========

void USekiroCombatComponent::StartAutoAttackWindow(float AnimDuration) {
  if (!GetWorld() || AnimDuration <= 0.0f)
    return;

  // 清除之前的攻擊窗口計時器
  StopAutoAttackWindow();

  float StartTime = AnimDuration * AttackWindowStartPercent;
  float EndTime = AnimDuration * AttackWindowEndPercent;
  float WindowDuration = EndTime - StartTime;

  if (WindowDuration <= 0.0f)
    return;

  // 在 StartTime 開啟攻擊窗口
  GetWorld()->GetTimerManager().SetTimer(
      AttackWindowStartHandle,
      [this, WindowDuration]() {
        // 重置命中標記
        ResetAttackHit();

        // 開始每幀執行攻擊判定
        GetWorld()->GetTimerManager().SetTimer(
            AttackWindowTickHandle,
            [this]() { PerformAttackHitCheck(); },
            0.016f, // ~60fps tick
            true    // repeating
        );

        // 在窗口結束時停止
        GetWorld()->GetTimerManager().SetTimer(
            AttackWindowEndHandle,
            [this]() { StopAutoAttackWindow(); },
            WindowDuration, false);
      },
      StartTime, false);
}

void USekiroCombatComponent::StopAutoAttackWindow() {
  if (GetWorld()) {
    GetWorld()->GetTimerManager().ClearTimer(AttackWindowStartHandle);
    GetWorld()->GetTimerManager().ClearTimer(AttackWindowTickHandle);
    GetWorld()->GetTimerManager().ClearTimer(AttackWindowEndHandle);
  }
}
