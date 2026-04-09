# Task.md — Arcane Souls: Rebirth
_最後更新：2026-04-09 17:48 (HKT)_

---

> ## ⚠️ AI 每次必讀提醒（每次 session 強制執行）
> 1. **必須先讀** `project_overview.md` + `unrealbullshit.md`，才能動手
> 2. **所有回答必須寫在此 task.md 裡，不在 chat 對話回答**
> 3. ❌ 禁止探索式 MCP calls
> 4. ❌ 禁止重複建 Python 腳本（先查 project_overview.md 清單）

---

## 🟡 問題 A：站姿後傾 — 等問題 C 解決後再調
## 🟡 問題 C：人物面向反了 — 待執行（改 VRMMesh Z Rotation = 180）
## ✅ 問題 B：武器掛載 — 已解決

---

## 🔴 新任務：Boss Arena 場景（日本神社）

---

## 📦 已掃描：othernewmodel 資產包

**Pack 名稱**：Stylized Japanese Shrine 5.0+  
**位置**：`othernewmodel/Stylized Japanese Shrine 5.0+/JapaneseShrine/`  
**狀態**：⚠️ 在 Content 資料夾外面，需要先 migrate 進去

### 包含內容

| 類別 | 資產 |
|------|------|
| **現成地圖** | `Level_Environment.umap`（完整室外神社場景）、`Level_AssetsOverview.umap`（資產預覽） |
| **神社建築** | ShrineBase×6、ShrineBody×6、ShrineCeiling×5、ShrineRoof×6、ShrineGate、ShrineStairs×2、ShrineFence×5 |
| **標誌性元素** | GateTorii（鳥居）、LampStone（石燈籠）、SacredRope（注連繩）、SaisenBox（賽錢箱）、EmaShinto（繪馬） |
| **室內元素** | RoomDivider（屏風）、ShrineLittle（小祠）+ 全套內部零件、Katana_Holder（刀架！）、LampPost |
| **自然環境** | TreeCedar×3款、TreeBamboo×2款、TreeBlackPine、Pond（水池）、RockCliff×5、StatueJizo×2 |
| **地面** | ShrineTileGround×11、StairsStoneFloor、DirtGround×2 |
| **小道具** | Well（水井）、Pot×22、Bench、Cart、Umbrella×2、Lantern | 

**資產總數：218 個 meshes**

---

## 🏗️ Import 計劃（第一步，必須先做）

### 方法：直接複製 uasset 資料夾進 Content

這些是 `.uasset` 格式（已是 UE5 原生格式），**不需要 import FBX**，直接 copy 進 Content 即可。

**步驟**：
```
1. 關閉 Unreal Editor（或者至少關閉所有相關 map）
2. 用 Windows Explorer 把以下資料夾
   從：othernewmodel/Stylized Japanese Shrine 5.0+/JapaneseShrine/
   複製到：Content/JapaneseShrine/
3. 重新開啟 Unreal Editor
4. Content Browser 應該能看到 Content/JapaneseShrine/ 資料夾
5. 打開 Content/JapaneseShrine/Maps/Level_Environment.umap 預覽
```

**⚠️ 注意**：
- 不要用 drag-and-drop 進 Editor（會觸發 reimport）
- 直接用 File Explorer copy 是最安全的方法
- 如果 Editor 提示 redirect，點 Fix Up Redirectors

---

## 🎯 場景方案分析：室內 vs 室外

### 方案一：室外場景（推薦，直接用 Level_Environment.umap）⭐⭐⭐

**優點**：
- 整個場景已經做好，直接開即用
- 有鳥居、石燈籠、池塘、杉樹、石牆——完整神社氣氛
- 開放空間，Spring Arm 相機不會穿牆
- Directional Light 空間充足

**缺點**：
- 可能需要清走一些雜物、縮窄 Arena 邊界
- 需要在場景中指定一個 Boss 戰鬥中心點

**Boss Arena 設計佈局（室外）**：
```
         [鳥居入口]
              |
    [石階 SM_ShrineStairs]
              |
    ══════════════════════
    ║   Boss Arena 中心   ║   ← 神社正殿前廣場
    ║   ~1500cm × 1500cm  ║
    ║      [靈夢 vs Boss] ║
    ══════════════════════
    [石燈籠]          [石燈籠]
    [圍欄邊界] ← SM_ShrineFence
         |
    [神社正殿背景]
```

### 方案二：室內場景（進階，需要自己拼接）⭐⭐

**用到的零件**：
- `SM_ShrineBody*` — 牆壁
- `SM_ShrineCeiling*` — 天花
- `SM_ShrineTileGround*` — 地板
- `SM_Lantern_01a` — 室內燈籠
- `SM_RoomDivider01a` — 屏風分隔
- `SM_Katana_Holder` — 刀架（完美配合戰鬥主題！）
- `SM_ShrineLittle*` — 小神龕裝飾

**缺點**：
- 需要手動拼接所有建築零件（3-5 小時）
- 室內相機容易穿牆（Spring Arm 要加 `Do Collision Test = false` 或收短 boom length）
- 光源複雜（要放多個 Point Light）

**結論：優先做室外，室外完成後視乎時間再做室內**

---

## 📋 執行計劃（按順序）

### Phase 1：Import 資產包 ✅ 你自己做
- [ ] 用 File Explorer 把 `JapaneseShrine` 資料夾 copy 進 `Content/`
- [ ] 開 UE5，確認 Content Browser 有 `/Game/JapaneseShrine/`
- [ ] 打開 `Level_Environment.umap` 確認場景正常顯示

### Phase 2：建立 Boss Arena Map（我可以用 MCP 幫你）
- [ ] 新建 Map：`Map_BossArena_Shrine`
- [ ] 把 Level_Environment 的場景遷移/參考進去（或直接在 Level_Environment 上改）
- [ ] 確定 Boss 戰鬥中心坐標（通常設 Origin = 0,0,0）
- [ ] 設置 Boxing 邊界：ShrineFence 圍起 Arena 周圍

### Phase 3：Player + Boss Spawn（我用 MCP 做）
- [ ] 放置 Player Start 在鳥居入口方向
- [ ] 放置 BP_SekiroEnemy 在神社正殿前（Boss 出場位置）
- [ ] 調整 Camera Boom Length（室外推薦 400-500）

### Phase 4：燈光氣氛（我用 MCP 做）
- [ ] Directional Light：黃昏/夜晚角度，偏橙紅色
- [ ] ExponentialHeightFog：輕霧，增加神祕感
- [ ] Point Lights：放在每個 SM_LampStone 石燈籠位置
- [ ] Sky Light：配合 Lumen

---

## ❓ 問你的問題（需要你決定）

1. **你想用室外還是室內場景？** → 建議室外（省時間）
2. **你想直接在 Level_Environment.umap 上改，還是新建一個 Map？** → 建議新建，保留原始場景
3. **Import 資產後，Unreal Editor 有沒有正常顯示 `/Game/JapaneseShrine/`？** → 做完告訴我，我再用 MCP 幫你放 Boss + 燈光

---

## bullshit 記錄

- lint errors（clang 報 undeclared identifier 等）= Antigravity clang 找不到 UE5 header，不影響 VS Build
- Fab.com listing 需要登入，AI 讀取 403，只能你自己確認價格

---

_資料截至：2026-04-09 17:48 (HKT)_
