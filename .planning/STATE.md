# STATE.md — FYP Boss Combat 當前進度

**Last updated:** 2026-04-20 13:00 HKT

---

## ✅ 當前狀態

**Phase 1 完成 ✅、Phase 2 完成 ✅ — 下一步：Phase 4 Merge to master**

---

## 📋 Phase 狀態

| Phase | 名稱 | 狀態 |
|-------|------|------|
| **Phase 1** | **Boss 危攻擊 AnimNotify** | `[x]` ✅ **完成，已 Push (74822ff)** |
| **Phase 2** | **玩家架勢條平衡** | `[x]` ✅ **完成，已 Push (e6084fe)** |
| Phase 3 | 玩家閃避系統 | `[ ]` TODO |
| **Phase 4** | **Merge to master** | `[ ]` 待處理 |

---

## ✅ 已完成工作

### Phase 1：危攻擊系統（74822ff）
- `SekiroEnemyAttributeComponent`：自身監聽 OnPerilousAttackStarted delegate
- BeginPlay 用 LoadClass 載入 WBP_PerilousWarning，1.5 秒後自動移除
- 5 個 Perilous Montage 加入 AnimNotify
- UAT 通過：危字UI ✅ / 不可格擋傷害 ✅ / Posture懲罰 ✅

### Phase 2：架勢條修復（e6084fe）
- `SekiroCombatComponent.cpp`：`AttackPostureDamage * 1.5f` → `PerfectParryPosturePenalty`（固定 15.0f）

---

## 🔲 下一步

**Phase 4：Merge feature/perilous-attack-notify 去 master**
- 高風險文件：SekiroHUD.h/.cpp（雙方都改過）
- 先在 GitHub 建 PR，確認是否有衝突

---

## Roadmap Evolution
- Phase 4 added: 處理 merge feature/perilous-attack-notify branches 去 master (2026-04-20)

---

## Git Branch 規則

```
master          ← 穩定版本
  └── feature/perilous-attack-notify  ← Phase 1 完成，待 merge
```

## Active Files

```
主要 C++        : SekiroEnemyAttributeComponent.h / .cpp
主要 Blueprint  : BP_SekiroEnemy / AM_Perilous_*
GSD 追蹤        : .planning/ROADMAP.md
```
