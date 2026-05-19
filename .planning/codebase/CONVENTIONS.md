# CONVENTIONS.md — 代碼慣例

## 命名規則

| 類別 | 前綴 | 範例 |
|------|------|------|
| Actor | `A` | `ASekiroCharacter` |
| Component | `U` | `USekiroCombatComponent` |
| Enum | `E` | `EParryResult` |
| Widget | `U` | `USekiroWidgetBase` |
| AnimNotify | `U` | `UAnimNotifyState_AttackWindow` |
| Blueprint | `BP_` | `BP_SekiroEnemy`, `BP_PlayerCharacter` |
| Widget BP | `WBP_` | `WBP_OVERHEAD`, `WBP_GameHUD` |
| AnimMontage | `AM_` | `AM_Perilous_Slash_Patchouli` |
| Input Action | `IA_` | `IA_Attack`, `IA_Block` |
| Niagara System | `NS_` | `NS_PerfectParrySpark` |
| Sound | 中文描述 | `Perfect_Parry_音效`, `處決聲音` |

## 組件通訊模式

1. **Delegate 廣播** (主要模式)
   ```cpp
   // 定義
   DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth);
   
   // 廣播
   OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
   
   // 綁定 (BeginPlay)
   AttributeComponent->OnDeath.AddDynamic(this, &ASekiroCharacter::OnDeath);
   ```

2. **FindComponentByClass** (跨 Actor 通訊)
   ```cpp
   USekiroDeflectComponent* DeflectComp = HitActor->FindComponentByClass<USekiroDeflectComponent>();
   ```

3. **GameplayTag** (攻擊類型標記)
   ```cpp
   FGameplayTag::RequestGameplayTag(FName("Attack.Perilous"))
   FGameplayTag::RequestGameplayTag(FName("Action.Attack.Light"))
   FGameplayTag::RequestGameplayTag(FName("State.Stunned"))
   ```

## 資源載入模式

### Constructor (ConstructorHelpers) — 永久資源
```cpp
static ConstructorHelpers::FObjectFinder<USoundBase> Asset(TEXT("/Game/path/asset.asset"));
if (Asset.Succeeded()) MemberVar = Asset.Object;
```
- 用途: 音效、粒子、CameraShake、InputAction
- 優點: 不會因 Blueprint 重置而丟失

### BeginPlay (StaticLoadObject) — 骨架相關資源
```cpp
UAnimMontage* M = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, *Path));
```
- 用途: ComboMontages, SpecialMontages（依骨架類型選擇）

### BeginPlay (StaticLoadClass) — Widget 類別
```cpp
UClass* WidgetCls = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/Game/WBP_OVERHEAD.WBP_OVERHEAD_C"));
```

## 攻擊判定模式

### 優先: AnimNotifyState 驅動
```
Montage 播放 → AnimNotifyState_AttackWindow
  NotifyBegin: ResetAttackHit()
  NotifyTick:  PerformAttackHitCheck() [每幀]
  NotifyEnd:   (無)
```

### Fallback: Timer 驅動
```
若 Montage 無 AttackWindow notify → StartAutoAttackWindow(Duration)
  Timer1 (30% 時間): 開始 → ResetAttackHit() + 開始 tick timer
  Timer2 (tick):     每 16ms → PerformAttackHitCheck()
  Timer3 (70% 時間): 結束 → StopAutoAttackWindow()
```

## 命中防重複

```cpp
bool bHasHit = false;  // 每次攻擊窗口開始重置
if (bHasHit) return;   // 已命中則跳過
bHasHit = true;        // 命中後設置
```

## 玩家/敵人區分

```cpp
// 判斷是否為玩家
if (Cast<APlayerController>(GetController())) { /* 玩家邏輯 */ }

// 或反向判斷（非玩家 = 敵人）
if (!Cast<APlayerController>(HitChar->GetController())) { /* AI 邏輯 */ }
```

## Debug 訊息慣例

```cpp
if (GEngine)
  GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("訊息"));

// 持久 Debug 訊息 (特定 key)
GEngine->AddOnScreenDebugMessage(10, 0.f, FColor::Green, DebugLine);
```

- **Cyan**: 系統資訊
- **Green**: 成功/確認
- **Yellow**: 警告/狀態
- **Orange**: AI 行為
- **Red**: 錯誤/危攻擊
- **Magenta**: 攻擊事件

## Timer 使用慣例

- `FTimerHandle` 成員變數保存，`ClearTimer()` + `Invalidate()` 清除
- Lambda capture 用 `TWeakObjectPtr` 避免 dangling pointer
- Boss 反擊延遲: `FMath::RandRange(0.3f, 0.7f)`

## 武器附加慣例

```
BlockWeaponPivot (USceneComponent)
  └── WeaponMesh (UStaticMeshComponent)

格擋時旋轉 Pivot (不是 WeaponMesh)，避免被動畫覆蓋
```
