# ARCHITECTURE.md — 系統架構

## 核心設計：Component-Based Actor

所有戰鬥邏輯通過 `UActorComponent` 掛載在 `ASekiroCharacter` 上，
玩家和 Boss 共用同一個 Character 類，透過 Controller 類型區分行為。

```
ASekiroCharacter (ACharacter)
├── USekiroAttributeComponent     — HP 管理
├── USekiroPostureComponent       — 架勢條 (累積/回復/崩)
├── USekiroDeflectComponent       — 格擋/彈刀判定
├── USekiroCombatComponent        — 攻擊/連招/處決/危攻擊
├── USekiroEnemyAttributeComponent — Boss AI 攻擊循環 (僅敵人)
├── UWidgetComponent (OverheadWidget) — 頭頂 HUD
├── UWidgetComponent (DeathblowWidget) — 處決提示
├── USpringArmComponent (CameraBoom)
├── UCameraComponent (FollowCamera)
├── USceneComponent (BlockWeaponPivot)
└── UStaticMeshComponent (WeaponMesh)
```

---

## 組件職責

### 1. USekiroAttributeComponent (36 行)
- **職責**: HP 管理 + 無敵狀態
- **關鍵屬性**: `MaxHealth=100`, `CurrentHealth`, `bIsInvincible`
- **Delegates**: `OnHealthChanged(Current, Max)`, `OnDeath()`
- **核心函數**: `ApplyDamage(Amount)` — 扣血前檢查無敵

### 2. USekiroPostureComponent (80 行)
- **職責**: 架勢條累積、自動回復、崩潰判定
- **關鍵屬性**: `MaxPosture=100`, `PostureRegenRateIdle=5`, `PostureRegenRateBlocking=15`
- **回復延遲**: `PostureRegenDelayAfterDamage=2s`
- **Delegates**: `OnPostureChanged(Current, Max)`, `OnPostureBroken()`
- **核心函數**: `AddPostureDamage(Amount)` — 累積架勢，滿時廣播 Broken
- **Tick**: 每幀回復架勢（格擋中回復更快）

### 3. USekiroDeflectComponent (93 行)
- **職責**: 格擋/彈刀判定 + Perilous 攻擊攔截
- **關鍵屬性**: `PerfectParryWindow=0.2s`, `bIsAI`, `DeflectProbability=0.3`, `BlockProbability=0.5`
- **Delegates**: `OnParryResult(EParryResult)`
- **核心函數**: `TryParry(GameplayTag)`
  - `Attack.Perilous` → 強制 `Failed`（無法格擋）
  - AI: 擲骰決定 Perfect/Blocked/Failed
  - 玩家: 按鍵時間差 ≤ PerfectParryWindow → Perfect, 否則 Blocked

### 4. USekiroCombatComponent (637 行) ⭐ 最複雜
- **職責**: 攻擊請求、連招系統、處決、危攻擊
- **連招系統**:
  - `ComboMontages[]` — 普通連招動畫（最多 4 段）
  - `SpecialMontages[]` — Boss 大招動畫
  - `ComboIndex`, `bCanCombo`, `bComboQueued` — 連招狀態管理
  - `ComboWindowTime` — 接招窗口時間
- **危攻擊**:
  - `PerilousAttackDamage=30`, `PerilousPostureDamage=25`
  - `PerformPerilousHitCheck()` — Attack.Perilous tag 繞過格擋
  - `PerfectParryPosturePenalty=15` — 精準擋刀固定懲罰
- **處決**: `RequestExecution()` → `CanExecuteTarget()` → `TryExecuteTarget()`
- **攻擊窗口**: 
  - 優先用 `AnimNotifyState_AttackWindow`
  - Fallback: Timer-based auto window (`StartAutoAttackWindow()`)
- **Delegates**: `OnAttackPerformed(Tag)`, `OnExecutionTriggered(Actor)`, `OnAttackStarted/Ended()`

### 5. USekiroEnemyAttributeComponent (310 行)
- **職責**: Boss AI 攻擊循環 + 危攻擊觸發
- **攻擊循環**:
  - `bAutoAttack=true`, `AttackInterval=3s`
  - `ComboAttackCount=3`, `ComboAttackInterval=0.5s`
  - Tick 計時 → `StartComboAttackCycle()` → 連續攻擊
- **危攻擊觸發**:
  - `TryPerilousAttack()` — 機率判定 (`PerilousAttackChance=0.3`)
  - `PerilousAttackMontages[]` — 危攻擊動畫
  - `PerilousAttackDamageMultiplier=2.0`
  - `OnPerilousAttackStarted` delegate → UI 顯示「危」字
- **面向玩家**: `bFacePlayer=true`, `FacePlayerRange=3000`

---

## 動畫系統

### AnimNotifyState_AttackWindow
- `NotifyBegin()` → `CombatComp->ResetAttackHit()` (重置命中標記)
- `NotifyTick()` → `CombatComp->PerformAttackHitCheck()` (每幀掃描)
- `NotifyEnd()` → 無操作

### Montage 自動載入 (BeginPlay)
- 依骨架類型自動載入對應 Montage:
  - Reimu → `Combo_Attack_03_0X_Seq_Montage_Reimu` x4
  - Patchouli → `Combo_Attack_01_0X_Seq_Montage_Patchouli` x4
  - Boss → `AM_Perilous_Slash_Patchouli`, `AM_Perilous_Thrust_Patchouli`

---

## UI 系統

### USekiroWidgetBase
- **用途**: 敵人頭頂 HUD (WBP_OVERHEAD)
- `BindToActor(AActor*)` — 綁定 HP/架勢條 delegates
- Blueprint Implementable: `UpdatePlayerPosture`, `UpdateEnemyPosture`, `UpdateHealth`, `UpdateEnemyHealth`

### USekiroGameHUDWidget
- **用途**: 玩家操作提示 HUD
- `FlashWidgetByName(FName)` — 按鍵時圖示閃爍
- Singleton pattern: `GetInstance()`

---

## 戰鬥流程圖

```
玩家按攻擊 → ASekiroCharacter::Attack()
  → CombatComponent->RequestExecution() [嘗試處決]
  → CombatComponent->RequestAttack()
    → Boss: RNG → SpecialMontages (危攻擊) or ComboMontages (普通連招)
    → AnimNotifyState_AttackWindow::NotifyTick()
      → PerformAttackHitCheck()
        → 目標 DeflectComp->TryParry(tag)
          → Perfect: 攻擊方吃架勢懲罰 + 火花 + 音效
          → Blocked: 防守方吃半架勢 + 火花 + 音效
          → Failed: 防守方吃血 + 架勢停止回復

Boss 危攻擊:
  EnemyAttrComp Tick → StartComboAttackCycle() → TryPerilousAttack()
    → 廣播 OnPerilousAttackStarted (危字 UI)
    → Timer 0.4s → Sphere Trace + Attack.Perilous tag
      → DeflectComp 強制 Failed → 無條件扣血扣架勢
```

---

## 鎖定系統 (Lock-On)

- `ToggleLockOn()` — 搜尋最近前方有 `LockOnTargetTag` 的 Actor
- Tick 中持續追蹤: 相機 + 角色面向目標
- Custom Depth Stencil 描邊效果
- 距離過遠自動解鎖

---

## 死亡流程

```
AttributeComponent->OnDeath.Broadcast()
  → ASekiroCharacter::OnDeath()
    1. 停止移動
    2. 停止 AI 攻擊 (bAutoAttack=false)
    3. 禁用玩家輸入
    4. 停止所有 Montage
    5. 停止格擋
    6. 重置連招
    7. 播放 DeathMontage → 凍結最後一幀 → 禁用碰撞
```
