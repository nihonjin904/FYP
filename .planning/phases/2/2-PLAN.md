---
wave: 2
depends_on: []
files_modified:
  - Source/FYP/Public/Components/SekiroCombatComponent.h
  - Source/FYP/Private/Components/SekiroCombatComponent.cpp
autonomous: true
---

# Phase 2: Boss Attack Automation Plan

## Tasks

<task>
<description>Inject SpecialMontages Array and Randomizer into Combat Component</description>
<read_first>Source/FYP/Public/Components/SekiroCombatComponent.h</read_first>
<action>
In `Source/FYP/Public/Components/SekiroCombatComponent.h`, locate the `// ========== COMBO 系統 ==========` section.

Add a new section for the boss abilities right before or after it:
```cpp
  // ========== 特殊大招攻擊 (Boss Special Skills) ==========
  
  // 存放特殊攻擊的 Montage 陣列 (Great Sword Slash, Upward Thrust 等)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|SpecialAttack")
  TArray<UAnimMontage *> SpecialMontages;

  // 每次攻擊時觸發特殊大招的機率 (預設 30%)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro|SpecialAttack", meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float SpecialAttackChance = 0.3f;
```
</action>
<acceptance_criteria>
- `Source/FYP/Public/Components/SekiroCombatComponent.h` contains `TArray<UAnimMontage *> SpecialMontages;`
- `Source/FYP/Public/Components/SekiroCombatComponent.h` contains `float SpecialAttackChance = 0.3f;`
</acceptance_criteria>
</task>

<task>
<description>Implement 30% Probability Overrides in RequestAttack()</description>
<read_first>Source/FYP/Private/Components/SekiroCombatComponent.cpp</read_first>
<action>
In `Source/FYP/Private/Components/SekiroCombatComponent.cpp`, find the `USekiroCombatComponent::RequestAttack()` function.
Navigate to the `if (!bIsAttacking) {` block (情況 1).

At the very top of `if (!bIsAttacking) {`, insert the override check:
```cpp
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

        auto MontageHasAttackWindow = [](UAnimMontage* M) -> bool {
          if (!M) return false;
          for (const FAnimNotifyEvent& N : M->Notifies) {
            if (N.NotifyStateClass &&
                N.NotifyStateClass->IsA(UAnimNotifyState_AttackWindow::StaticClass()))
              return true;
          }
          return false;
        };

        if (!MontageHasAttackWindow(SpecialMontages[RandomIndex])) {
          StartAutoAttackWindow(Duration); // fallback
        }

        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &USekiroCombatComponent::OnMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, SpecialMontages[RandomIndex]);
        
        return; // 直接搶斷，終止下方 ComboMontages 執行
      }
      // --- END BOSS RNG OVERRIDE ---
```
Ensure the original `if (ComboMontages.IsValidIndex(0) && ComboMontages[0]) {` immediately follows this block, safely serving as the fallback sequence if the RNG fails or `SpecialMontages` is empty.
</action>
<acceptance_criteria>
- `Source/FYP/Private/Components/SekiroCombatComponent.cpp` contains `if (SpecialMontages.Num() > 0 && FMath::FRand() <= SpecialAttackChance)`
- `FMath::RandRange` is used to index into `SpecialMontages`
- The method uses an early `return;` inside the special attack block to abort the normal `ComboMontages` fallback.
</acceptance_criteria>
</task>

## Verification
<must_haves>
- Successfully compiles in IDE.
- Exposed properties allow Blueprint arrays to equip "Great Sword Slash" and "Upward Thrust".
</must_haves>
