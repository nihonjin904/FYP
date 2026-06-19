# DEPENDENCIES.md — 組件依賴關係

## 組件依賴圖

```
ASekiroCharacter
│
├── USekiroAttributeComponent ← 無依賴（最底層）
│     ↑ OnHealthChanged → UI 更新
│     ↑ OnDeath → ASekiroCharacter::OnDeath()
│
├── USekiroPostureComponent ← 無依賴
│     ↑ OnPostureChanged → UI 更新
│     ↑ OnPostureBroken → ASekiroCharacter::OnPostureBroken()
│
├── USekiroDeflectComponent ← GameplayTags
│     ↑ OnParryResult → ASekiroCharacter::HandleParryResult()
│     │                  (音效/火花/Camera Shake/Hit Stop)
│
├── USekiroCombatComponent ← DeflectComp, PostureComp, AttributeComp, EnemyAttrComp
│     │ 讀取目標的 Deflect/Posture/Attribute 組件
│     │ 讀取自身的 Posture 組件 (精準擋刀懲罰)
│     ↑ OnAttackPerformed → (未使用外部)
│     ↑ OnExecutionTriggered → ASekiroCharacter::OnExecutionTriggered()
│     ↑ OnAttackStarted/Ended → ASekiroCharacter (武器拖尾)
│
└── USekiroEnemyAttributeComponent ← CombatComp (僅 Boss)
      │ BeginPlay: FindComponentByClass<CombatComponent>
      │ Tick: 計時 → StartComboAttackCycle() → SekiroChar->Attack()
      │ TryPerilousAttack() → 播放 Montage + Sphere Trace
      ↑ OnPerilousAttackStarted → BP_SekiroEnemy (危字 UI)
```

## 跨 Actor 依賴 (攻擊判定時)

```
Attacker (Boss)                          Target (Player)
─────────────────                        ─────────────────
CombatComponent                          
  PerformAttackHitCheck()  ──Trace──→   DeflectComponent.TryParry(tag)
                                         PostureComponent.AddPostureDamage()
                                         AttributeComponent.ApplyDamage()
```

## UI 依賴

```
WBP_OVERHEAD (USekiroWidgetBase)
  ← BindToActor() in SekiroCharacter::BeginPlay
  ← AttributeComponent.OnHealthChanged → UpdateHealth()
  ← PostureComponent.OnPostureChanged → UpdateEnemyPosture()

WBP_GameHUD (USekiroGameHUDWidget)
  ← SekiroCharacter::Attack() → FlashWidgetByName("Img_Attack")
  ← SekiroCharacter::StartBlock() → FlashWidgetByName("Img_Block")
  ← SekiroCharacter::ToggleLockOn() → FlashWidgetByName("Img_LockOn")
```

## AnimNotify 依賴

```
AnimNotifyState_AttackWindow
  → CombatComponent.ResetAttackHit()
  → CombatComponent.PerformAttackHitCheck()

Montage 結束回調:
  → CombatComponent.OnMontageEnded() (連招重置)
  → EnemyAttrComp.OnPerilousAttackMontageEnded() (危攻擊結束)
```

## 文件依賴矩陣

| 文件 | 依賴 |
|------|------|
| SekiroCharacter.h/cpp | 所有 Component, UI, Niagara, Sound, EnhancedInput |
| SekiroCombatComponent | AttributeComp, PostureComp, DeflectComp, EnemyAttrComp, AnimNotify |
| SekiroEnemyAttributeComponent | CombatComp, DeflectComp, AttributeComp, PostureComp |
| SekiroDeflectComponent | GameplayTags（獨立） |
| SekiroPostureComponent | GameplayTags（獨立） |
| SekiroAttributeComponent | 無依賴（最獨立） |
| AnimNotifyState_AttackWindow | CombatComp |
| SekiroWidgetBase | AttributeComp, PostureComp |
| SekiroGameHUDWidget | 無 C++ 依賴（Singleton） |

## 第三方模組依賴

| 模組 | 用途 | 必要性 |
|------|------|--------|
| EnhancedInput | 玩家輸入 (IA_Attack 等) | ⭐ 必要 |
| GameplayTags | 攻擊類型、狀態標記 | ⭐ 必要 |
| Niagara | 火花粒子 | 可選（降級為無特效） |
| MotionWarping | 處決動畫位移 | 可選（目前概念階段） |
| UMG | 全部 UI | ⭐ 必要 |
