# Task.md — Arcane Souls: Rebirth
_最後更新：2026-04-12 23:32 (HKT) 星期日_

---

> ## 🔴 AI 必讀規則 🔴
> 你每次要查看 project_overview.md unrealbullshit.md 每次都要實時更新
> 每次回答我要在 task.md 裡面回答我 並且更新
> 請你不要在chat對話裡面和我說 要直接在task.md 裡面全部回答我
> 請先查清楚問題 / 先plan 讓我批准先
> MCP 操作後必須用 analyze_blueprint_graph 驗證
> **用戶說「讓我批准先」= 必須等用戶明確在對話中說「批准」或「做」才能執行**
> **model 有些是日文名字，小心**

---

## ✅ 已完成

### 火花調大 — 已編譯通過 ✅

```diff
# SekiroCombatComponent.cpp
Perfect Parry 火花：FVector(2.5f) → FVector(6.0f)
Block 火花：FVector(1.5f) → FVector(4.0f)
```

---

## 🔴 新發現：Boss 格擋率太低！

### 你截圖裡看到的實際值

| 屬性 | Blueprint 實際值 | 我改的 C++ 預設值 | 差異 |
|---|---|---|---|
| Block Probability | **0.3** | 0.55 | ❌ Blueprint 覆蓋了 C++ |
| Deflect Probability | **0.05** | 0.35 | ❌ Blueprint 覆蓋了 C++ |
| **總格擋率** | **35%** | 90% | ❌ 太低！65% 攻擊直接砍到 Boss |

### 為什麼會這樣？

我改的是 **C++ 預設值**（.h 文件），但 `BP_SekiroEnemy` Blueprint 裡面已經**序列化**(serialized)了舊的值（0.3 和 0.05）。Blueprint 的值**永遠覆蓋 C++ 預設值**。

這就是你說「Boss 沒有擋刀」的原因 — 只有 35% 格擋率，大部分攻擊都直接砍到了。

### 修復方法（不需要改代碼）

**直接在 Blueprint 面板改數值** — 你剛才的截圖位置就是：

1. 打開 `BP_SekiroEnemy` → Components → 點擊 **Deflect Component**
2. 右邊 Details 面板 → **Sekiro > AI**
3. 把 `Block Probability` 改成 **0.55**
4. 把 `Deflect Probability` 改成 **0.35**
5. 左上角 **Compile** → **Save**

改完後格擋率 = 0.55 + 0.35 = **90%**

| 改前 | 改後 |
|---|---|
| Block: 0.3 → 30% 普通格擋 | Block: 0.55 → 55% 普通格擋 |
| Deflect: 0.05 → 5% 彈刀 | Deflect: 0.35 → 35% 彈刀 |
| 總格擋: 35% | 總格擋: **90%** ✅ |
| 被砍到: 65% | 被砍到: 10% |

### 風險
- 🟢 零風險（只是改 Blueprint 數值，不改代碼）
- ❌ 不和朋友衝突
- ⚠️ 如果 90% 太高可以調低，這些數值隨時能改

---

## 📋 你現在要做的步驟

### Step 1：改 Blueprint 格擋率（5 秒）
1. `BP_SekiroEnemy` → Components → **Deflect Component (DeflectComponent)**
2. Details → Sekiro > AI
3. `Block Probability` → 改成 **0.55**
4. `Deflect Probability` → 改成 **0.35**
5. 左上角 **Compile** 按鈕 → 再按 **Save** 按鈕

### Step 2：重啟 UE Editor（因為改了 .cpp）
- 關掉 UE Editor
- 重新打開（或者刪 `Binaries/Win64/*patch*` 後重開）

### Step 3：進遊戲測試
- 鎖定 Boss → 連續普通攻擊
- 預期效果：
  - 90% 攻擊被擋 → 看到「PERFECT PARRY!」或「Blocked!」+ **大火花** + 音效
  - Boss 格擋後 0.3-0.7 秒自動反擊
  - 只有 10% 攻擊砍到 Boss

---

## 📊 全部已改動的文件

| 文件 | 改了什麼 | 狀態 |
|---|---|---|
| `SekiroDeflectComponent.h` | 預設值 0.55/0.35（但被 BP 覆蓋） | ✅ 已編譯 |
| `SekiroCombatComponent.cpp` | 対刀邏輯 + 火花 6.0f/4.0f | ✅ 已編譯 |
| `SekiroCharacter.cpp` | **沒改** | ✅ 安全 |
| `BP_SekiroEnemy` | **你手動改**格擋率 | ⏳ 等你做 |

---

_回答時間：2026-04-12 23:32:26 (HKT) 星期日_
_累積對話 tokens：約 250,000_
