# Task.md — Arcane Souls: Rebirth
_最後更新：2026-04-13 00:34 (HKT) 星期一_

---

> ## 🔴 AI 必讀規則 🔴
> 你每次要查看 `project_overview.md` `unrealbullshit.md` 每次都要實時更新
> 每次回答我要在 `task.md` 裡面回答我 並且更新
> 請你不要在chat對話裡面和我說 要直接在task.md 裡面全部回答我
> 我操你媽的 你task.md 完全沒有更新 還是舊的數據和對話和回答
> 然後model 有些是 日文名字 這個部分 請你小心
> 請先查清楚問題 / 先plan 讓我批准先
> **用戶說「讓我批准先」= 必須等用戶明確在對話中說「批准」或「做」才能執行**
> **model 有些是日文名字，小心**

---

> ## ⚠️ 朋友工作範圍（不可衝突）
> 朋友正在做：
> 1. **Checkpoint 左上角 UI 小地圖**
> 2. **Checkpoint 傳送功能**
> 
> 我們改的範圍：`SekiroCombatComponent.cpp/h`、`SekiroEnemyAttributeComponent.cpp/h`、`SekiroDeflectComponent.cpp/h`
> 朋友可能改的範圍：`SekiroCharacter.cpp/h`（Input 綁定、UI 部分）
> **結論：P1-P4 全部改 Component 文件，✅ 不衝突。P5 Dodge 需要改 SekiroCharacter.cpp，⚠️ 可能衝突。**

---

## ✅ P1：降低精準彈刀的架勢懲罰 — 已完成 ✅

**狀態**：✅ 代碼已修改，等待 Build + 測試

**改了什麼**：
- **文件**：`SekiroCombatComponent.cpp` 第 420 行
- **改動**：`AttackPostureDamage * 3.0f` → `AttackPostureDamage * 1.5f`

**Diff**：
```diff
- MyPosture->AddPostureDamage(AttackPostureDamage * 3.0f);
+ MyPosture->AddPostureDamage(AttackPostureDamage * 1.5f);
```

**效果**：
- 之前：被完美彈刀 → 攻擊者扣 `20 * 3.0 = 60` 架勢 → **2 次**就爆架勢（MaxPosture=100）
- 現在：被完美彈刀 → 攻擊者扣 `20 * 1.5 = 30` 架勢 → **約 4 次**才爆架勢

**風險**：0%（只改了一個數字）
**和朋友衝突**：❌ 不衝突（只改 SekiroCombatComponent.cpp）

---

## 🔨 下一步：Build

你需要在 Visual Studio 或 UE Editor 編譯：

**方法 1**：UE Editor Live Coding → **Ctrl+Alt+F11**
**方法 2**：Visual Studio → **Ctrl+Shift+B**（確認 Configuration 是 `Development Editor`）

Build 成功後，Play 測試：
1. 讓 Boss 攻擊你
2. 精準格擋（Perfect Parry）Boss 的攻擊
3. 確認螢幕左上出現 `PERFECT PARRY!` 黃字
4. 確認 Boss 架勢條增長幅度比之前小（需要 ~4 次精準彈刀才爆，而不是 2 次）

---

## 📋 剩餘 Plan（等你批准後再做）

| 優先級 | 問題 | 改動文件 | 和朋友衝突？ | 狀態 |
|---|---|---|---|---|
| ~~P1~~ | ~~精準彈刀扣太多架勢 (3.0f→1.5f)~~ | ~~SekiroCombatComponent.cpp~~ | ❌ | ✅ 已完成 |
| P2 | Boss 擋完卡住不動 | SekiroCombatComponent.cpp + .h | ❌ 不衝突 | ✅ 用戶已批准，等 P1 測試通過後執行 |
| P3 | 格擋有但沒火花 | 需要 debug | ❌ 不衝突 | 等批准 |
| P4 | Perilous Attack 沒有傷害 | SekiroEnemyAttributeComponent.cpp | ❌ 不衝突 | 等批准 |
| P5 | Shift 閃避 | SekiroCharacter.cpp/h | ⚠️ 要問朋友 | 等確認 |

---

## ❓ 等你回覆

1. **P1 Build + 測試**：Build 成功了嗎？測試結果怎樣？
2. **P2 執行**：P1 測試 OK 後要直接做 P2 嗎？（Boss 擋完卡住不動的修復）

---

_回答時間：2026-04-13 00:34:25 (HKT) 星期一_
_累積對話 tokens：約 45,000_
