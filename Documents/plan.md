# Arcane Souls: Rebirth — FYP 開發計劃 (plan.md)
> 最後更新：2026-04-07 | 負責：Lam Chi Him (kelvelam6)

---

## 🎯 目前階段目標（Current Sprint Goal）

把以下核心系統全部跑通成為一個可玩的 Vertical Slice：

| # | 目標 | 優先級 | 狀態 |
|---|---|---|---|
| 1 | 玩家角色模型 + 基本移動 | 🔴 必須 | ⚙️ 進行中 — Reimu Model 已 apply，修正斜站/武器/動作問題中 |
| 2 | Boss 怪物模型整合 | 🔴 必須 | ✅ BP_SekiroEnemy Model 已成功 apply |
| 3 | Boss 四種攻擊動作 (25%各觸發) | 🔴 必須 | 需製作 AM + AI 隨機選擇邏輯 |
| 4 | 主場地一個（Boss Arena） | 🔴 必須 | 已有 Map_CombatDemo，需美化 |
| 5 | 開場對話 + AI Gen 人物立繪 | 🟡 重要 | 需 AI 生圖 + UMG 對話 UI |
| 6 | 第二個場地（選做） | 🟢 選做 | 時間充裕才做 |

---

## 📋 詳細任務分解

---

### MODULE 1：玩家角色模型 (Player Character)

**目標**：選定一個玩家角色模型，綁定完整動作 + 戰鬥系統

**已有資產**：
- `Content/Characters/reimu/` — 博麗靈夢
- `Content/Characters/remilia/` — 蕾米莉亞
- `Content/Characters/patchouli/` — 帕秋莉
- `BP_魔_博麗_霊夢.uasset` — 靈夢的 Blueprint（已有基礎）
- `ABP_SekiroCharacter.uasset` — 現有動畫藍圖

**待辦**：
- [ ] 確認使用哪個角色作為 Player（建議靈夢，Blueprint 最完整）
- [ ] 綁定 Animation Blueprint 到所選角色 Mesh
- [ ] 確認 Parry / Dodge / Attack / Death 等基礎動作已有 Montage
- [ ] 綁定 `BP_SekiroCharacter` 戰鬥邏輯到所選角色

**負責**：Kelvin

---

### MODULE 2：Boss 怪物模型 (Boss Enemy Model)

**目標**：整合 Boss 模型到 UE5，能夠正常 Spawn 並運行 AI

**建議模型來源（優先順序）**：
1. **FAB（原 Marketplace）** — 找 Japanese/Yokai 風格 Boss（可直接 UE5 相容）
2. **Mixamo** — 免費骨架 + 動作，需要轉換 Root Motion
3. **自製 / 已有 Sekiro 模型** — 可考慮用 `Characters/Sekiro/` 資料夾現有資產

**待辦**：
- [ ] 決定 Boss 主題（建議：鬼/妖狐/影武者等 和風 Boss）
- [ ] 下載或取得 Boss 骨架 Mesh（.fbx 格式）
- [ ] Import 到 `Content/Characters/Boss/` 目錄
- [ ] 設定 Physics Asset + Capsule Collision
- [ ] 建立 `ABP_Boss` 動畫藍圖
- [ ] 建立 `BP_Boss` 繼承 `BP_SekiroEnemy`

**注意事項**：
- Boss Mesh 骨架需與 UE5 Mannequin 骨架相容，或使用 IK Retargeter
- 若用 Mixamo：須在 Mixamo 選「In Place」動作，匯出 FBX for UE4/5

**負責**：Kelvin + CK

---

### MODULE 3：Boss 四種攻擊動作 (4 Attack Animations)

**目標**：Boss 具備 4 種不同攻擊，各以 25% 機率隨機觸發

**四種攻擊設計建議**：

| 動作 ID | 攻擊名稱 | 傷害類型 | 攻擊特性 | 觸發機率 |
|---|---|---|---|---|
| `Attack_A` | 重劈 (Heavy Slam) | 大傷害 + 削韌 | 長前搖，可格擋 | 25% |
| `Attack_B` | 快速橫斬 (Sweep) | 中傷害 | 短前搖，需側閃 | 25% |
| `Attack_C` | 刺擊 (Thrust) | 中傷害 + 架勢傷害 | 可彈反觸發處決機會 | 25% |
| `Attack_D` | 連擊 (Combo 2-hit) | 小+大傷害 | 兩段，第二下有延遲 | 25% |

**UE5 實作方式**：
```
// Behavior Tree → BTTask_SelectAttack
1. BTTask 隨機產生 1-4 (FMath::RandRange)
2. 根據結果設定 Blackboard Key: "SelectedAttack"
3. BTTask_PlayMontage 播放對應 AM_Boss_Attack_X
4. 動作結束後回到 Patrol/Chase 狀態
```

**待辦**：
- [ ] 製作或取得 4 個 Boss 攻擊 Animation Montage
  - `AM_Boss_Attack_A` (Heavy Slam)
  - `AM_Boss_Attack_B` (Sweep)
  - `AM_Boss_Attack_C` (Thrust)
  - `AM_Boss_Attack_D` (Combo)
- [ ] 每個 Montage 加入 AnimNotify：`AN_EnableHitbox` / `AN_DisableHitbox`
- [ ] 建立 `BTTask_SelectAttack` Blueprint（隨機邏輯）
- [ ] 在 Behavior Tree 整合隨機攻擊選擇
- [ ] 設定攻擊 Cooldown（建議 1.5-2.5 秒，避免連招過快）
- [ ] 測試每種攻擊的 Hit Detection 是否正確

**負責**：CK（AI/BT） + Kelvin（動作整合）

---

### MODULE 4：主場地 (Boss Arena Level)

**目標**：一個完整、美觀的 Boss 戰場地

**目前狀態**：
- `Content/Maps/Map_CombatDemo.umap` — 基礎戰鬥 Demo 已有
- `Content/Nanite_Env_Bundle_1/` — 環境資產包已有

**場地設計方向**：
- 主題：**影祠本殿（Shadow Shrine Main Hall）**
- 風格：日式神社 + 夜晚 + 月光 + 紅色燈籠
- 規模：中等大小圓形/方形 Arena（半徑約 20m）
- 照明：Lumen 全局光 + 多個點光源（燈籠）+ 月光方向光

**待辦**：
- [ ] 以 `Map_CombatDemo` 為基礎或新建 `Map_BossArena`
- [ ] 鋪設地板 + 圍牆（使用 `Nanite_Env_Bundle_1` 資產）
- [ ] 加入燈籠、鳥居、石柱等裝飾物
- [ ] 設定 Sky Atmosphere + HDRI Backdrop（夜晚天空）
- [ ] 設定 Directional Light（月光，角度 15-30°）
- [ ] Post Process Volume：輕微 Bloom + Color Grading（偏藍紫冷色調）
- [ ] 設定 Player Start + Boss Spawn Point
- [ ] 確認場地邊界（Blocking Volume，防止走出地圖）
- [ ] 效能測試：確保 60 FPS on GTX 1660S

**（選做）第二個場地**：
- [ ] 霧鎖参道（Fogbound Sando）
- [ ] 主題：霧中竹林小徑，半開放式場景
- [ ] 時間充裕時才做

**負責**：Kelvin

---

### MODULE 5：開場對話 + AI 生成人物立繪 (Opening Cutscene & AI Art)

**目標**：Boss 戰開始前有劇情對話，配合 AI 生成角色立繪

**流程設計**：
```
[進入場地] → [開場對白 UI 彈出] → [顯示人物立繪] → [對話逐字顯示] 
→ [玩家按鍵跳過/繼續] → [對話結束] → [戰鬥開始 + Boss Spawn]
```

**對話內容建議（草稿）**：
```
靈夢：「你就是守護此地的惡靈……」
Boss：「闖入聖域者，唯有以血償還。」
靈夢：「我沒有退路了。」
Boss：「那就以你的靈魂，為此地獻祭！」
[戰鬥開始]
```

**AI 生成人物立繪方案**：
- 工具：Stable Diffusion / Midjourney / DALL-E
- 風格：Anime-style，半身立繪（bust shot），白色/透明背景
- 需要生成：
  - 玩家角色立繪（靈夢 — 戰鬥表情）
  - Boss 立繪（對應 Boss 造型）
  - 各 2-3 個不同表情版本

**UE5 UMG 實作**：
- [ ] 建立 `WBP_OpeningDialogue` Widget Blueprint
  - 左側：玩家角色立繪 (Image)
  - 右側：Boss 立繪 (Image)
  - 底部：對話框 (Border + TextBlock，Rich Text 逐字顯示)
  - 角色名稱標籤
  - 「按任意鍵繼續」提示
- [ ] 建立 `BP_CutsceneManager` 負責控制對話序列
  - 陣列存儲對話行（`TArray<FDialogueLine>`）
  - 每行包含：說話角色、文字、立繪表情版本
- [ ] 對話結束後：Fade Out UI → 淡入 → Boss Spawn 並開始戰鬥
- [ ] AI 生成立繪圖片 import 到 `Content/UI/Cutscene/` 目錄

**待辦**：
- [ ] 撰寫完整對話腳本（3-6 行）
- [ ] 用 AI 工具生成人物立繪
  - 靈夢（戰鬥表情）x2
  - Boss（威脅表情）x2
- [ ] Import 立繪到 UE5（PNG，推薦 2:3 比例，如 512x768）
- [ ] 建立 `WBP_OpeningDialogue` Widget
- [ ] 建立 `BP_CutsceneManager`
- [ ] Level Blueprint 整合：進場觸發對話，對話完觸發 Boss

**負責**：Kelvin（UI/Art） + CK（Level BP 整合）

---

## 📅 估計時間線

```
Week 1 (Module 1+2)：玩家角色確認 + Boss 模型 Import + 初步整合
Week 2 (Module 3)  ：4 種 Boss 攻擊 Montage + BT 隨機邏輯
Week 3 (Module 4)  ：Boss Arena 場地美化 + 照明設定
Week 4 (Module 5)  ：開場對話 UI + AI 立繪生成 + 整合測試
Week 5 (Buffer)    ：Bug Fix + 效能優化 + （選做）第二場地
```

---

## 🏗️ 建議目錄結構

```
Content/
├── Characters/
│   ├── reimu/           (玩家角色)
│   ├── Boss/            (待建立)
│   │   ├── SK_Boss.uasset
│   │   ├── ABP_Boss.uasset
│   │   └── AM_Boss_Attack_A/B/C/D.uasset
├── Maps/
│   ├── Map_BossArena.umap    (主 Boss 場地)
│   └── Map_FogboundSando.umap (選做)
├── UI/
│   ├── WBP_HUD.uasset        (已有)
│   ├── WBP_OpeningDialogue.uasset (待建立)
│   └── Cutscene/
│       ├── T_Reimu_Battle.png
│       ├── T_Boss_Angry.png
│       └── ...
├── Blueprints/
│   ├── BP_Boss.uasset         (待建立)
│   ├── BP_CutsceneManager.uasset (待建立)
│   └── BTTask_SelectAttack.uasset (待建立)
```

---

## 💡 Antigravity 建議（技術風險 + 解決方案）

### ⚠️ 風險 1：Boss 模型骨架不相容
- **問題**：外部 Boss 模型骨架與 UE5 Mannequin 不同，動作無法直接重用
- **解決**：使用 UE5 IK Retargeter 將現有動作遷移到 Boss 骨架，或直接找 UE5 Mannequin 骨架的 Boss 動作包

### ⚠️ 風險 2：AI 生成立繪風格不一致
- **問題**：靈夢和 Boss 的 AI 生成圖片風格可能差距太大
- **解決**：使用同一個 Stable Diffusion LoRA 模型，或統一用 Midjourney 同一 style 參數（`--style raw --ar 2:3`）

### ⚠️ 風險 3：開場對話打斷玩家輸入
- **問題**：對話期間玩家可能移動或攻擊，破壞體驗
- **解決**：對話期間 `DisableInput()`，Set Pawn 為 Spectator 模式，對話完成後 `EnableInput()` + Boss Spawn

### ⚠️ 風險 4：場地效能
- **問題**：燈籠、神社裝飾 + Lumen 可能在 GTX 1660S 拉低幀率
- **解決**：限制動態光源數量（≤4個動態點光），靜態裝飾用 Static GI，開啟 Hardware Ray Tracing 門檻設置

### 💡 額外建議
- **Audio**：建議加入 Boss 出場音效（日式鼓聲）+ 對話前奏音樂，大幅提升代入感
- **Camera**：開場對話可做一個簡單的 Cinematic Camera Shake（輕微震動），讓 Boss 出場感更強烈
- **Deathblow**：若時間允許，替 Boss 設計一個「Final Strike」處決動畫，完成感大增

---

## ✅ 驗收標準 (Acceptance Criteria)

- [ ] 玩家可以正常操控角色移動、攻擊、格擋
- [ ] Boss 能隨機觸發四種攻擊（各約 25%）
- [ ] 開場對話正確顯示，按鍵可進行下一行，結束後戰鬥開始
- [ ] AI 生成立繪在 UI 中正確顯示，風格統一
- [ ] 場地在 GTX 1660S 維持 ≥ 60 FPS
- [ ] 整個流程：進入地圖 → 對話 → 戰鬥 → Boss 死亡，流暢無卡頓

---

*plan.md by Kelvin Lam (kelvelam6) | Arcane Souls: Rebirth | FYP 2025-2026*

---

## 🚨 緊急優先修正（2026-04-07）

### ⚠️ GitHub 協作規則（必須遵守）
- **不可亂改朋友也在改的共用文件**（特別是 `SekiroCharacter.h` / `SekiroCharacter.cpp`）
- 如需改 C++ 核心文件，必須先確認朋友當前的工作分支，或使用 Branch + PR 流程
- 優先用 Blueprint/Component 方式解決，減少對共用 C++ 文件的依賴

### 🔧 當前狀態（已確認）
- ✅ 主角 Reimu（`BP_SekiroCharacter`）Model 已成功 apply
- ✅ 敵人 Boss（`BP_SekiroEnemy`）Model 已成功 apply

### 🔴 待修問題（按優先順序）

| # | 問題 | 原因 | 解決方向 | 狀態 |
|---|---|---|---|---|
| A | 人物斜著站（身體/頭部歪） | VRM rest pose 與 Mannequin 不同，retarget 後偏差 | 先把 VRMMesh Roll 改回 0°；如仍歪則調 `RTG__魔_博麗_霊夢` | 🔧 未解決 |
| B | 刀拿的位置不對 | WeaponMesh 掛在 Mannequin `hand_r`，Reimu 手位置不重合 | Runtime C++ re-attach 至 VRM `J_Bip_R_Hand` | 🔧 未解決 |
| C | 擋刀（Block）動作不對 | Block animation/pivot 跟著 Mannequin 骨架，Reimu 骨架位置偏 | 同問題 B，刀位修好後一並驗證 | 🔧 未解決 |
| D | 跑步時拿刀不對 | 同問題 B，武器 attach 點錯 | 同問題 B | 🔧 未解決 |

### 📋 行動計劃
1. **【Editor，無需改 C++】** 打開 `BP_SekiroCharacter` → 選 VRMMesh Component → Details → 把 Rotation Roll 改為 0° → Compile → Play 測試斜站問題
2. **【Editor，無需改 C++】** 如果仍斜，打開 `RTG__魔_博麗_霊夢`（IKRetargeter）手動調 spine/head chain mapping
3. **【需改 C++】** 在 `SekiroCharacter.cpp` BeginPlay() 加 runtime re-attach 代碼（確認朋友分支後才動）→ rebuild → 調整武器 offset
4. 修好後做全動作驗證：Idle / Walk / Run / Attack / Block / Dodge / Death

> 詳細技術分析見 `task.md`
