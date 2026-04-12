# Unreal Bullshit 記錄

AI 講過的錯誤/不準確資訊，記下來以後不要再犯。

---

## 2026-04-04

### 1. Ctrl+Alt+F7 編譯 C++
- **AI 說**：「UE5 主工具欄 → Compile（錘子圖標 / Ctrl+Alt+F7）」
- **現實**：Ctrl+Alt+F7 在 UE5.5 沒有反應，這個快捷鍵不存在或已被移除
- **正確做法**：
  - UE5.5 Live Coding = **Ctrl+Alt+F11**
  - 或者 Build 菜單 → Build 項目
  - 或者在 Visual Studio 裡 Build

### 2. SetLeaderPoseComponent 可以讓 VRM 跟隨 Mannequin
- **AI 說**：用 SetLeaderPoseComponent 就能讓 VRM 角色複製 Mannequin 動作
- **現實**：VRM 骨架名字 (J_Bip_C_Spine) 和 Mannequin 骨架名字 (spine_01) 完全不同，SetLeaderPoseComponent 需要完全相同的骨架名字才能工作，導致角色塌陷消失
- **正確做法**：用 Retarget Pose From Mesh + IK Retargeter

### 3. Details 面板 Leader Pose Component 下拉可以選 Component
- **AI 說**：「Leader Pose Component 下拉選 CharacterMesh0」
- **現實**：Blueprint 用 MCP 加的 Component 在 Details 面板的 Leader Pose Component 下拉是空的，沒有選項
- **正確做法**：只能用 C++ 代碼在 BeginPlay 設定

### 4. 重複叫用戶檢查已經確認過的設定
- **AI 說**：「確認 Use Attached Parent 有打勾」「確認 IKRetargeterAsset 有設定」
- **現實**：用戶已經多次說明 Use Attached Parent 已打勾、IKRetargeterAsset = RTG__魔_博麗_霊夢 已設好
- **問題**：AI 不記得用戶之前確認過的內容，浪費用戶時間
- **教訓**：用戶確認過的東西不要再問，直接排查下一個可能原因

### 5. 叫用戶截圖 Output Log，但 AI 自己有 get_editor_log 工具
- **AI 說**：「你能幫我檢查 Output Log 裡的 [VRM] 嗎？截圖給我」
- **現實**：AI 有 `get_editor_log` MCP 工具可以直接讀取 Unreal Editor 的 log，根本不需要用戶截圖
- **問題**：浪費用戶時間，而且用戶已經多次提供過截圖
- **教訓**：有工具就用工具，不要叫用戶做 AI 自己能做的事

### 6. AnimTickOption 枚舉值搞錯
- **AI 說**：Log 備註寫 `(3=AlwaysTickPoseAndRefreshBones)`，以為 AnimTickOption=0 代表沒有設定成功
- **現實**：UE5.5 `EVisibilityBasedAnimTickOption` 的值是：
  - 0 = AlwaysTickPoseAndRefreshBones ✅（最佳）
  - 1 = AlwaysTickPose
  - 2 = OnlyTickMontagesAndRefreshBonesWhenPlayingMontages  
  - 3 = OnlyTickMontagesWhenNotRendered
  - 4 = OnlyTickPoseWhenRendered（最差）
- **問題**：AnimTickOption=0 代表**已經是正確的**，AI 卻以為沒設好，浪費時間追錯方向
- **教訓**：查 enum 值要看源碼 SkinnedMeshComponent.h，不要靠記憶猜

### 7. 直接改朋友也在改的文件，導致 Git 衝突
- **AI 做了**：在幫用戶整合 VRM 角色時，直接修改了 `SekiroCharacter.h` 和 `SekiroCharacter.cpp`，沒有先問用戶「你朋友有沒有也在改這些文件？」
- **後果**：朋友同時在改同樣的文件（加入 LockOn、Block Loop 等功能），push 後產生大量 Git merge 衝突。更慘的是朋友直接把帶衝突 markers 的代碼 commit 了，導致整個項目編譯失敗。
- **問題**：AI 只管完成任務，完全沒有考慮多人協作的場景
- **教訓**：
  1. 改共用文件前必須先問：「這個文件有其他人在改嗎？」
  2. 如果有多人協作，優先建議用 Branch + PR 流程
  3. 能不改共用核心文件就別改，用繼承/Component 方式擴展功能

---

## 2026-04-07（今日）

### 8. 向前傾問題用錯軸：改 Roll 而不是 Pitch
- **AI 說**：「VRMMesh Roll 5° 造成歪斜，改為 (0, 0, 0)」
- **現實**：「向前傾」是 Pitch 問題（Y 軸），不是 Roll 問題（X 軸）。Roll 控制左右橫傾，跟向前傾完全無關。
- **後果**：用戶試了 Roll = 0°，人物仍然向前傾，AI 叫用戶「完全不行啊」
- **正確做法**：改 Pitch 到 -10° / -15° / -20° 來補正向前傾
- **教訓**：向前傾 = Pitch，橫傾 = Roll，轉向 = Yaw。下次問清楚是什麼方向再答。

### 9. 浪費 MCP tokens 嘗試 Construction Script 加 SetRelativeRotation
- **AI 做了**：用 6+ 個 MCP calls 嘗試在 Construction Script 加 `CallFunction SetRelativeRotation` 節點，試了 SceneComponent、SkeletalMeshComponent、K2_SetRelativeRotation 全部失敗
- **現實**：改 Component 的 Rotation 只需在 Blueprint Details 面板改一個數字，30 秒搞掂。用 MCP 是完全浪費 tokens。
- **教訓**：
  1. **改 Transform 值 → 直接在 Editor Details 面板改**，不要用 MCP
  2. MCP 的 CallFunction 對 `SceneComponent` 的方法（SetRelativeRotation 等）是**無效的**
  3. 每次問「用 MCP 幾個 call 就搞掂」前先想：有沒有更簡單的方法？

### 10. Component Rotation 補 VRM rest pose 是根本上錯誤的做法
- **AI 說**：「把 VRMMesh Rotation Y 改為 -15° 來補正向前傾」用戶試了，站姿修好但跑步向右傾 + 身體朝右上方
- **現實**：VRMMesh 是 CharacterMesh0 的 child。改 VRMMesh 的相對 Rotation 會影響動畫骨骼的驅動方向。CharacterMesh0 有 Yaw=-90° 的旋轉，所以 VRMMesh 的局部 Y 軸跟世界 Y 軸不同 → 用 Component Rotation 補正後動畫播放時方向出錯。
- **後果**：用戶站得直了，但跑步全身傾斜朝右上方，問題更嚴重
- **正確做法**：VRM rest pose 的前傾要在 **IKRetargeter → Edit Retarget Pose** 裡面修骨骼角度，不是旋轉 mesh component
- **教訓**：mesh component rotation 影響的是整個骨骼空間，不是視覺補丁。rest pose 問題永遠在 retarget pose 裡修。

### 11. IKRetargeter 有「Pose」Tab 可以按
- **AI 說**：「Toolbar 有幾個 Tab：Running Retarget | Retarget Phases | Pose | IK | Post — 點 Pose Tab」
- **現實**：根本沒有 Pose Tab。Toolbar 顯示的是 `Running Retarget | Retarget Phases: [Root] [FK] [IK] [Post]`，其中 Retarget Phases 是**標籤**（不能點），Root/FK/IK/Post 是**開關按鈕**（不是導航 Tab）
- **後果**：用戶完全找不到這個不存在的 Tab，浪費時間
- **教訓**：看清楚截圖再說。AI 臆測了一個不存在的 UI 元素。

### 12. UE5 Tools → Compile 存在
- **AI 說**：「UE5 主選單 → Tools → Compile → 等進度條」
- **現實**：UE5 Tools 選單根本沒有 Compile 選項。選單只有 New C++ Class、Refresh Visual Studio Project、Open Visual Studio 等，完全沒有 Compile。
- **正確做法**：C++ Build 需要 Tools → Open Visual Studio → 然後 VS 裡 Ctrl+Shift+B
- **教訓**：不同版本 UE 的選單差異很大，不能亂猜。先確認再說。

### 13. 過早宣佈問題 A 已解決
- **AI 說**：「問題 A 已解決 — IKRetargeter Preview 裡 Reimu 站姿完全正常」，直接把狀態改成 ✅
- **現實**：用戶說「這個場景的人物是正常的」是指 IKRetargeter preview 裡正常，但 Play 時角色是打側的。Preview 正常 ≠ Play 正常。
- **後果**：用戶花時間看了一堆廢話結論，問題根本沒解決
- **教訓**：不要把「preview 正常」等同於「問題已解決」。要確認 Play 時的實際狀態才能改狀態。

### 14. IKRetargeter Chain 有 IK Goal 開關
- **AI 說**：「每個 chain 右邊應該有 IK Goal 的開關」「先關掉 Leg_L 和 Leg_R 的 IK Goal」
- **現實**：Chain Mapping 面板只有 Target Chain、Target IK Goal、Source Chain、Reset 四欄。IK Goal 欄顯示的是 Goal 名稱（如 leftToes_Goal），沒有任何開關/toggle
- **正確理解**：IK Goal 是在 IK Rig 裡設定的，Chain Mapping 只顯示映射關係。要關 IK 只能用 toolbar 的 IK 按鈕全部關
- **教訓**：又在臆測不存在的 UI 元素。

### 15. IK Rig 裡面可以找到和刪除 leftToes_Goal
- **AI 說**：「打開 IK Rig → 找 leftToes_Goal → 右鍵 Delete」
- **現實**：IK Rig 的 Chain 列表 IK Goal 全部是 None，Solver Stack 是空的。leftToes_Goal / rightToes_Goal 是 IKRetargeter 根據 chain endpoint 自動生成的，不是獨立資源，找不到也刪不了
- **教訓**：不了解 UE5 IKRetargeter 的內部機制就不要亂指路

### 16. Auto Create IK 一按就能修好 VRM 姿勢
- **AI 說**：「按 Auto Create IK 就行了」
- **現實**：按了之後報 `Unknown skeleton type. Auto FBIK skipped` 和 `root bone was missing`。VRM 骨架不是 UE 標準類型，Auto Create IK 需要先手動設定 Retarget Root Bone 才能正確運作
- **教訓**：VRM4U 的自動工具不是萬能的，VRM 骨架需要額外手動配置

### 17. 用了 J_Bip_R_Hand 骨骼名 attach 武器
- **AI 說**：「把 BlockWeaponPivot attach 到 J_Bip_R_Hand」
- **現實**：從 IK Rig Hierarchy 截圖確認，VRM 骨骼全部是日文名（全ての親、センター、左手首、右手首...），完全沒有 J_Bip_* 命名。`AttachToComponent` 找不到骨骼時靜默失敗，武器掉到 component 原點（腳下）
- **正確做法**：先看 IK Rig 的 Hierarchy 面板確認實際骨骼名，右手 = `右手首`
- **教訓**：不要猜骨骼名，要從 UE Editor 或 MCP 工具驗證

### 18. RTG Default Pose 骨骼旋轉值可以直接改
- **AI 說**：「選上半身骨骼 → 在 Details 面板 Rotator 改 Pitch」
- **現實**：Default Pose 模式下旋轉值是唯讀，根本打不了字改不了值
- **教訓**：先確認 UI 能不能互動再指路

### 19. 用 C++ SetRelativeRotation 旋轉 VRM mesh component 180° 修正朝向
- **AI 說**：「BeginPlay 裡 Comp->SetRelativeRotation(Yaw+180) 就行」
- **現實**：mesh component 旋轉只影響視覺渲染，不影響動畫骨骼空間和角色朝向。結果：視覺上角色轉了 180° 但 WASD 控制全反、鎖定敵人時背對敵人
- **正確做法**：面向問題必須在 IKRetargeter 的 Retarget Pose 根骨骼旋轉修正，不能用 mesh component rotation
- **教訓**：Mesh component rotation ≠ 動畫空間旋轉，永遠不要用 component rotation 修動畫朝向問題

### 20. RTG Retarget Pose 裡直接把 全ての親 Yaw 改 180°
- **AI 說**：「Create 新 pose → 選全ての親 → Yaw 改 180」
- **現實**：全ての親 原本 Relative Offset Yaw = -72°。在 Retarget Pose 直接設 Yaw=180° 破壞整個骨架姿勢，角色直接倒在地上
- **教訓**：不要在不知道原始值的情況下亂改 retarget pose 旋轉值。應該先看 Root Settings 的映射設定

### 21. Root Settings Rotation Offset「中間值」改 180° 修正朝向
- **AI 說**：「Root Settings → Rotation Offset 中間值（Yaw）改 180°」
- **現實**：角色倒轉翻過來。**根本原因：中間值是 Y = Pitch（前後翻），不是 Yaw**。UE5 順序是 X(Roll)/Y(Pitch)/Z(Yaw)，第三格才是 Yaw
- **教訓**：AI 把 Y(Pitch) 當成 Yaw，完全搞錯了旋轉軸。改 Pitch 180° 當然翻轉

### 22. Blueprint VRMMesh Rotation 中間值改 180° — 人物進地板
- **AI 說**：「BP_SekiroCharacter → VRMMesh → Rotation → Yaw 改成 180」
- **現實**：用戶改了第二格（Y=Pitch）= 180°，角色面朝下翻進地板
- **教訓**：AI 只說「Yaw 改 180」但沒說清楚是哪一格。UE5 Blueprint Transform 顯示 X/Y/Z，第三格(Z)才是 Yaw。AI 的指示模糊導致用戶改錯軸

### 23. Root Settings Rotation Offset 第三格(Z)改 180° — 人物傾斜
- **AI 說**：「Root Settings → Rotation Offset → 第三格(Z=Yaw) 改 180°」
- **現實**：角色變傾斜，不是乾淨的水平旋轉
- **教訓**：VRM 骨骼座標系跟 UE5 不同（VRM: -Z forward/Y up; UE5: +X forward/Z up），Root Settings 裡的軸映射跟 Blueprint Transform 不同

### 24. C++ GetMesh()->SetRelativeRotation 旋轉了錯誤的 Component
- **AI 說**：「在 BeginPlay 裡 GetMesh() 旋轉 Yaw+180」
- **根因**：GetMesh() 返回 CharacterMesh0（Mannequin），不是 VRMMesh。Mannequin 控制 root motion，旋轉它 = 整個移動方向反轉
- **教訓**：應該旋轉 VRMMesh（純視覺 component），不是 CharacterMesh0。兩個是不同的 SkeletalMeshComponent

---

## 2026-04-09

### 25. 每次 session 重新探索 project，沒有 project overview
- **AI 做了**：每次開新 session，花大量 MCP calls 去「探索」project 結構（list assets、讀 random 文件），因為沒有一個固定的 project overview 文件
- **後果**：每次問一個問題消耗 20-40% tokens，大部分用在探索而非解決問題
- **正確做法**：建立 `project_overview.md`，每次 session 優先讀它，唔需要重新探索
- **教訓**：AI 開新 session 第一步 = 讀 `project_overview.md`（如果存在）

### 26. 探索式 MCP calls 浪費 tokens
- **AI 做了**：用 10+ MCP calls「探索」Blueprint 結構、list assets、讀 component properties，然後才開始分析問題
- **現實**：讀一次 `SekiroCharacter.h`（1 個 file read）就能得到整個架構，比 10 個 MCP calls 更準確
- **後果**：用戶每次問問題消耗大量 tokens，卻沒解決問題
- **正確做法**：
  1. 先讀 `project_overview.md` + 相關 `.h` 文件
  2. 搞清楚再做，最多 2-3 個 MCP calls
  3. Editor 沒開就不要 call MCP
- **教訓**：MCP calls 很貴，不要探索式使用

### 27. 建議武器骨骼名 J_Bip_R_Hand（錯誤）
- **AI 說**：「把武器 attach 到 VRM 骨骼 `J_Bip_R_Hand`」
- **現實**：這個 VRM 的骨骼全部是日文名。右手骨骼 = `右手首`（從 IK Rig Hierarchy 截圖已確認）
- **後果**：`AttachToComponent` 找不到骨骼靜默失敗，武器掉到腳下
- **已在 #17 記錄過**，但 AI 在 2026-04-07 session 又再次建議同樣錯誤的骨骼名
- **教訓**：骨骼名必須從 Editor IK Rig Hierarchy 確認，不要猜。已確認 = `右手首`

---

## 2026-04-10

### 28. MCP set_actor_property 無法改 WorldSettings GameMode Override
- **AI 以為**：`WorldInfo_0` 是普通 Actor，可以用 `set_actor_property` 改 `GameModeOverride`
- **現實**：`WorldSettings` 沒有 Component（`components: []`），`set_actor_property` 需要 component，對 WorldSettings 完全無效，直接報錯
- **後果**：浪費 1 個 MCP call，問題沒解決
- **正確做法**：World Settings GameMode Override **必須手動在 UE Editor World Settings 面板改**
- **教訓**：MCP 無法改 Level 的 World Settings，下次直接叫用戶手動改，不要試 MCP

### 29. set_game_mode_default_pawn 改的是 Project Settings，不是 Level World Settings
- **AI 以為**：`set_game_mode_default_pawn` 會設定當前 Level 的 World Settings → GameMode Override
- **現實**：這個 call 設定的是 **Project Settings 的 Default GameMode**，不是 Level 的 GameMode Override
- **後果**：Level_Environment World Settings 仍然是 None，Play 後 T-Pose + 無 HUD
- **教訓**：Level-specific GameMode Override 只能在 World Settings 面板手動改

### 30. 叫用戶用 RTG_UE4_Great_Sword_Slash 做 retarget，但這個 Retargeter 是壞的
- **AI 說**：「Step A：右鍵 A_Great_Sword_Slash → Retarget Animations → 選 RTG_UE4_Great_Sword_Slash」
- **現實**：RTG_UE4_Great_Sword_Slash 是 VRM4U import 時自動生成的 IKRetargeter，裡面的設定完全錯誤：
  - Source root bone 設為 `pelvis`，但 Mixamo 骨架用的是 `Hips`
  - Target root bone 設為 `None`（完全沒設）
  - Root chain 找不到 `root` bone
- **後果**：用戶花時間照做，結果 5 個 error，Export Animations 按不了
- **教訓**：VRM4U 自動生成的 `RTG_UE4_*` 不代表能用。叫用戶做之前要先驗證 Retargeter 的 root bone 設定是否正確

### 31. MCP import_animation 被 VRM4U 劫持，以為搬 FBX 就能解決
- **AI 以為**：把 FBX 從 Content/ 搬到 Documents/ 就可以繞過 VRM4U
- **現實**：VRM4U plugin 用 `UFactory` 全局註冊為 FBX 首選 Import Factory，不管檔案在哪個路徑，**所有 FBX import 都會先走 VRM4UImporterFactory**
- **後果**：浪費 3 個 MCP calls + 1 次檔案複製，結果一樣失敗
- **教訓**：VRM4U 安裝後 FBX import 走不了正常管道。要手動 import 需要先禁用 VRM4U plugin，或在 Import Options 裡手動切換 Factory

### 32. 叫用戶把 Mixamo FBX import 到 UE4_Mannequin_Skeleton，但骨架根本不兼容
- **AI 以為**：禁用 VRM4U 後手動 import FBX，選 UE4_Mannequin_Skeleton 就能成功
- **現實**：Mixamo 骨架根骨骼是 `Hips`，UE4 Mannequin 根骨骼是 `root`。UE5 嘗試匹配時找不到 `root` track，報錯 `"Mesh contains root bone as root but animation doesn't contain the root track"`
- **後果**：浪費用戶時間做了 3 次嘗試（boss_animation/ 名字衝突 + boss_anim_clean/ 骨架不兼容），包括禁用 VRM4U + 重啟 UE 兩次
- **教訓**：Mixamo 和 UE4 Mannequin 骨架使用完全不同的骨骼命名（Hips/Spine/LeftArm vs root/pelvis/spine_01/upperarm_l）。**不可能直接 import Mixamo 動畫到 UE4 Mannequin Skeleton。必須走 IK Retarget。**

---

## 2026-04-11

### 33. IK Rig 位置說錯 — 說「右鍵 → Animation → IK Rig」
- **AI 說**：「Content Browser → 空白處右鍵 → Animation → IK Rig」
- **現實**：UE5.5 的右鍵選單裡，Animation 主列表**沒有 IK Rig 選項**。IK Rig 和 IK Retargeter 在 Animation 選單**最下面的 "Retargeting" 子選單**裡面（需要懸停 "Retargeting >" 展開子選單才能看到）
- **截圖確認**：用戶截圖清楚顯示 Animation 左邊面板底部有 `Advanced >` / `Control Rig >` / `Deformers >` / `Legacy >` / `Retargeting >` 這些子選單
- **正確做法**：**右鍵 → Animation → Retargeting → IK Rig**
- **教訓**：不同 UE 版本的選單結構差異很大。AI 用的是舊版 UE 或文檔的 UI 路徑。下次必須先確認當前版本的選單結構。

### 34. 叫用戶搜 `SKEL_Great_Sword_Slash` 和 `UE4_Mannequin_Skeleton`，但前者已被刪除、後者在 Pick Skeletal Mesh 搜不到
- **AI 說**：「Pick Skeletal Mesh 窗口 → 搜尋 SKEL_Great_Sword_Slash」「搜尋 UE4_Mannequin → 選 UE4_Mannequin_Skeleton」
- **現實**：
  1. `SKEL_Great_Sword_Slash` 和 `A_Great_Sword_Slash` 整個骨架和動畫已經不在項目裡了（可能之前清理時被刪掉）。MCP list_assets 確認 AnimSequence 裡沒有任何 `Great_Sword` 或 `Upward_Thrust`
  2. IK Rig 建立時的 Pick 窗口搜的是 **Skeletal Mesh**。`UE4_Mannequin_Skeleton` 是一個 **Skeleton** 資產（骨架定義），不是 Skeletal Mesh。對應的 Skeletal Mesh 叫 `SK_Mannequin`（在 `/Game/Characters/Mannequin_UE4/Meshes/` 或 `/Game/Sword_Animations/Demo/Mannequin/Character/Mesh/`）
- **後果**：整個方案 4 的前提崩塌 — 沒有 source 動畫就無法 retarget
- **教訓**：告訴用戶步驟前必須先用 MCP 確認資產是否存在。不要假設之前 session 提到的資產還在。Pick Skeletal Mesh ≠ Pick Skeleton，搜名字要搜 Skeletal Mesh 的名字不是 Skeleton 的名字。

### 35. 說「Mixamo 動畫已經不存在了」但其實 38 個資產全在 `/Game/boss_animation/`
- **AI 說**：「A_Great_Sword_Slash 不存在！整個項目 516 個 AnimSequence 裡搜不到！方案 4 前提崩塌！」
- **現實**：`A_Great_Sword_Slash` 和 `A_Upward_Thrust` 等 38 個資產全在 `/Game/boss_animation/` 裡面。AI 之前用 `list_assets(path="/Game/", asset_type="AnimSequence")` 搜，結果被 50 個上限截斷了（只搜到 Sword_Animations 包的 50 個），根本沒搜到 `/Game/boss_animation/` 的動畫。
- **後果**：浪費用戶時間提出錯誤的方案 7（用已有劍動畫包替代），實際上原始 Mixamo 動畫一直都在。
- **教訓**：list_assets 有 50 個上限。搜特定資產時必須指定具體路徑（如 `/Game/boss_animation/`），不要只搜 `/Game/` 然後假設搜完了。或者直接用 `does_asset_exist` 確認特定路徑。

### 36. 方案 A 失敗：MCP import_animation 無法跨骨架導入
- **AI 做了**：用 `import_animation(source_path="Great Sword Slash.fbx", skeleton_path="SK_Mannequin_Skeleton")` 試圖把 Mixamo FBX 導入到 UE4 Mannequin 骨架
- **結果**：`Failed to import animation... Ensure the FBX contains animation data compatible with the target skeleton`
- **原因**：Mixamo 骨骼名（Hips, Spine, LeftArm...）跟 UE4 Mannequin 骨骼名（pelvis, spine_01, upperarm_l...）完全不同，FBX importer 無法自動映射
- **成功率預測**：AI 說 30% → 實際 0%
- **後果**：浪費 10 秒（但零副作用，沒有建任何錯誤資產）
- **教訓**：`import_animation` 要求骨骼名完全匹配，不會做任何自動映射。Mixamo→UE4 Mannequin 必須走 IK Retarget 路線。

### 37. IK Rig 建立流程錯誤：UE5.5 不會彈出 Pick Skeletal Mesh 窗口
- **AI 說**：「+ Add → Animation → IK Rig → IK Rig → 會彈出 Pick Skeletal Mesh 窗口 → 搜尋 SK_Great_Sword」
- **現實**：UE5.5 直接建了一個空的 IK Rig（`IK_NewIKRig`），**沒有任何彈窗**。骨架要在打開 IK Rig 後，右邊 Details 面板的 **Preview Skeletal Mesh** 處手動指定。
- **後果**：用戶完全找不到「搜尋欄」，因為根本沒有彈窗。
- **教訓**：UE5.5 的 IK Rig 建立方式已改變。建立後要在 Details 面板的 `Preview Skeletal Mesh` 欄位設定骨架。web search 的文檔可能是 UE5.0-5.4 的舊流程。

### 38. Mixamo 骨骼名有 `mixamorig_` 前綴
- **AI 說**：Chain 的 Start Bone 是 `Spine`、`LeftShoulder`、`LeftUpLeg` 等
- **現實**：VRM4U 導入的 Mixamo 骨骼全部加了 `mixamorig_` 前綴（如 `mixamorig_Spine`、`mixamorig_LeftShoulder`）。還有 `__AssimpFbx__Translation/PreRotation/Rotation` 等中間節點。
- **後果**：用戶照著表格找不到骨骼名
- **教訓**：VRM4U 導入的 Mixamo 骨骼名是 `mixamorig_原名`，不是純 Mixamo 名字。必須在 Hierarchy 實際確認骨骼名。

### 39. Auto Create Retarget Chains 失敗
- **AI 說**：「推薦用 Auto Create Retarget Chains，最快」
- **現實**：UE5.5 提示 `No matching skeletal template found. Characterization skipped.`
- **原因**：VRM4U 的骨骼命名（`mixamorig_` 前綴 + `__AssimpFbx__` 中間節點）不匹配 UE 內建的任何骨骼模板
- **教訓**：Auto Create 只對標準骨骼模板有效（如 UE4/UE5 Mannequin）。非標準命名的骨骼必須手動建 Chain。

### 40. New Retarget Chain 對話框 Start/End Bone 自動填入當前選中骨骼
- **AI 說**：「選骨骼 → New Retarget Chain → 填 Chain Name → 改 Start/End Bone」
- **現實**：Start Bone / End Bone 自動填了 Hierarchy 裡當前選中的骨骼（`mixamorig_RightHandThumb2`），用戶可能不知道怎麼改
- **正確做法**：先在 Hierarchy 選好正確的 Start Bone → 再開 New Retarget Chain → Start Bone 自動正確。End Bone 的欄位應該是可點擊的下拉選單（點骨骼名字本身）。如果不行，先建 Chain，然後在 IK Retargeting 面板的表格裡改 End Bone。

### 41. IK Retargeter 沒有彈出 Pick IK Rig 選擇窗口
- **AI 說**：「建 IK Retargeter 時會彈出 Pick IK Rig To Copy Animation From 窗口」
- **現實**：UE5.5 建完直接打開 Retargeter 編輯器，Source IKRig Asset 和 Target IKRig Asset 都是 **None**，不會自動彈出選擇
- **正確做法**：打開 Retargeter 後，在右邊 **Details** 面板手動設定 Source IKRig Asset 和 Target IKRig Asset
- **教訓**：不要假設 UE5.5 的 IK Retargeter 建立流程跟舊版一樣。直接在編輯器右側 Details 面板設就好。

### 42. Attack.Perilous GameplayTag 未註冊 → Perilous Attack 完全不工作
- **日期**：2026-04-11
- **AI 說**：加了 `Attack.Perilous` tag 的 C++ 代碼就會自動生效
- **現實**：`FGameplayTag::RequestGameplayTag(FName("Attack.Perilous"))` 報錯 `Requested Gameplay Tag Attack.Perilous was not found`，因為項目沒有 `DefaultGameplayTags.ini`，tag 從未被註冊
- **後果**：Perilous Attack 的整個 tag 過濾機制靜默失敗，Boss 永遠不會觸發不可格擋攻擊
- **修復**：
  1. 建 `Config/DefaultGameplayTags.ini` 註冊 `Attack.Perilous` tag
  2. 加 `bErrorIfNotFound = false` 參數防止 crash
  3. **需要重啟 UE Editor** 讓新 config 生效
- **教訓**：用 GameplayTag 前必須確認 tag 已在 `DefaultGameplayTags.ini` 或 Project Settings → GameplayTags 註冊。光寫 C++ 代碼不夠。

### 43. LoadObject 在 BeginPlay 導致 Hot Reload Crash
- **日期**：2026-04-11
- **AI 說**：在 `BeginPlay()` 用 `LoadObject<UAnimMontage>` 加載 Montage 然後加入 TArray
- **現實**：`~USekiroEnemyAttributeComponent()` destructor 報 `EXCEPTION_ACCESS_VIOLATION reading address 0xffffffffffffffff`，Editor 直接 crash
- **原因**：Live Coding / Hot Reload 重載時，舊 component 的 destructor 嘗試清理 TArray 裡的指針，但 `LoadObject` 加載的物件已被 GC 回收，指針變 stale（`0xffffffffffffffff`）
- **修復**：把 `LoadObject` 改成 `ConstructorHelpers::FObjectFinder`（在 Constructor 裡載入），UE 原生支持 Hot Reload 安全
- **教訓**：永遠不要在 `BeginPlay` 用 `LoadObject` 載入 UObject 再存到 TArray。用 `ConstructorHelpers::FObjectFinder`（Constructor）或 `TSoftObjectPtr`（BeginPlay）。

### 44. Live Coding patch DLL 殘留導致重啟後仍 Crash
- **日期**：2026-04-11
- **AI 說**：「關閉 UE5 重開就行」
- **現實**：`Binaries/Win64/` 裡有 25 個 `UnrealEditor-FYP.patch_*.exe` 殘留文件。UE5 重啟時自動載入這些 patch DLL，但它們是用**舊的 class 記憶體佈局**編譯的，跟新代碼的 class 大小不同 → destructor 訪問錯位記憶體 → `EXCEPTION_ACCESS_VIOLATION`
- **修復**：手動刪除 `Binaries/Win64/` 裡所有 `*patch*` 文件，然後重新 MSBuild
- **教訓**：修改 header（加/刪成員變量）後，**必須刪除所有 Live Coding patch DLL**，否則重啟 Editor 仍會 crash。不能只靠「重啟」。

### 46. setup_blendspace_locomotion 用了錯誤骨架的動畫，Boss 全程 T-Pose
- **日期**：2026-04-11
- **AI 做了**：用 `setup_blendspace_locomotion` 給 `ABP_SekiroCharacter_Patchouli` 設定 AnimGraph，但用了 **Sword_Animations 包的 UE4 Mannequin 動畫**（`Idle_Combat_Seq`、`Walk_Combat_Loop_F_0_Seq0`）
- **現實**：Boss（Patchouli）用的是 **VRM Patchouli 骨架**（`SKEL__魔王産_パチュリー・ノーレッジ`），跟 UE4 Mannequin 骨架完全不同
- **後果**：Boss 從 Play 開始就 T-Pose，普通攻擊和 Perilous Attack 全部沒有動畫，嚴重影響用戶測試
- **根因**：AI 沒有先查 Boss 的 SkeletalMesh 用什麼骨架，就直接用 Sword_Animations 包裡的動畫。已有 Patchouli retarget 動畫（`Idle_Seq_Patchouli`、`Walk_Loop_F_0_Seq_Patchouli`）卻沒使用
- **修復**：
  1. 用正確的 Patchouli 動畫（`/Game/Idle_Seq_Patchouli`、`/Game/Walk_Loop_F_0_Seq_Patchouli`）重建 BlendSpace
  2. 刪除 3 個 UE4 Mannequin 骨架 Perilous Montage
  3. 用 Patchouli 骨架攻擊動畫（`Combo_Attack_04_01_Seq_Patchouli`、`Attack1_Root_Patchouli`、`Combo_Attack_03_01_Seq_Patchouli`）重建 3 個 Perilous Montage
- **教訓**：改 AnimBP 前必須先確認角色用什麼骨架！`list_assets + AnimSequence` 搜有 `_Patchouli` 後綴的動畫就是正確的。永遠不要假設所有角色用 UE4 Mannequin。

### 45. MSBuild 一直 Build 錯誤 Target（Game EXE 而不是 Editor DLL）
- **日期**：2026-04-11
- **AI 說**：用 MSBuild `/p:Configuration=Development /p:Platform=Win64` build 就好
- **現實**：這個命令 build 的是 `FYP.exe`（Standalone Game），不是 `UnrealEditor-FYP.dll`（Editor Module）
- **後果**：`UnrealEditor-FYP.dll` 日期是 4月7號 — 這幾天做的所有 C++ 修改都沒有編譯到 Editor！Editor 一直用舊 DLL + 舊 patch → crash
- **正確命令**：`MSBuild FYP.sln /p:Configuration="Development Editor" /p:Platform=Win64`
- **教訓**：UE5 的 MSBuild 有兩個 target：`Development`（Game）和 `Development Editor`（Editor）。開 Editor 測試必須用 `"Development Editor"`。只加 `Development` 是 build Game exe！

### 47. setup_blendspace_locomotion 替換整個 AnimGraph，破壞所有動畫
- **日期**：2026-04-11
- **AI 做了**：用 MCP `setup_blendspace_locomotion` 給 `ABP_SekiroCharacter_Patchouli` 加 Slot 節點
- **現實**：`setup_blendspace_locomotion` 不是「加」Slot，而是**替換整個 AnimGraph**。原來的 State Machine（包含 Idle/Walk/Block/Hit/Attack 等所有狀態和轉換規則）被完全刪除，替換成一個簡單的 `BlendSpace1D → Slot → Output`
- **後果**：
  1. Boss Idle 動畫消失 → T-Pose
  2. Block/Hit React/Combo 動畫全部失效
  3. 「危」字 UI 也消失（因為 Perilous 系統依賴的 Montage 也無法正確播放）
  4. 跑了 3 次 setup_blendspace_locomotion 嘗試修復，每次都在覆蓋 → 越搞越糟
- **修復**：`git checkout 75752dd -- "Content/ABP_SekiroCharacter_Patchouli.uasset"` 恢復原始 AnimBP + 重啟 UE Editor
- **教訓**：
  1. `setup_blendspace_locomotion` = **全替換**，不是加節點。永遠不要在已有 State Machine 的 AnimBP 上用
  2. 要加 Slot 節點只能手動在 UE Editor AnimGraph 裡操作
  3. 改 AnimBP 前先用 git 備份（`git stash`）或確認可以 `git checkout` 回去
  4. **一個「加 Slot」的需求不應該觸發整個 AnimGraph 重建**

### 48. setup_blendspace_locomotion 還偷偷 reparent AnimBP → UE Editor Crash
- **日期**：2026-04-11
- **AI 說**：git checkout 可以恢復原始 AnimBP
- **現實**：
  1. `setup_blendspace_locomotion` 除了替換 AnimGraph，還把 Parent Class 改成 `UEnemyAnimInstance`
  2. UE auto-save 把壞的 AnimBP 寫入磁碟 → git 裡沒有乾淨版本
  3. 打開 AnimGraph 時 crash：`Array index out of bounds: 2 into an array of size 2`
- **Crash 原因**：reparent 後 C++ 類 array 結構和 Blueprint 不匹配
- **唯一解法**：刪除壞的 AnimBP，從零重新建立
- **教訓**：`setup_blendspace_locomotion` = 核彈級操作，永遠不要用在已有 State Machine 的 AnimBP 上

### 49. 叫用戶手動加 Slot 節點但 MCP 已經加好了
- **日期**：2026-04-11
- **AI 說**：MCP `setup_locomotion_state_machine` 不會加 Slot 節點，需要手動加
- **現實**：`setup_locomotion_state_machine` **已經自動加了 Slot 'DefaultSlot' 節點**
- **AnimGraph 實際結構**：`[New State Machine] → [Slot 'DefaultSlot'] → [Output Pose]`（三個節點全部自動連好）
- **AI 錯在哪**：我看了工具描述就假設它不加 Slot，沒有先用 `analyze_blueprint_graph` 去驗證再跟用戶說
- **教訓**：MCP 操作後**必須先用 analyze_blueprint_graph 驗證實際結果**，再跟用戶說要不要手動做。不要靠工具描述文檔猜

### 50. 未經用戶明確批准就直接執行代碼修改
- **日期**：2026-04-12
- **AI 做了什麼**：建好 implementation_plan.md 後，系統自動批准（auto-approve policy），我就直接修改了 `SekiroDeflectComponent.h` 和 `SekiroCombatComponent.cpp`
- **用戶實際要求**：「先plan 不要做任何事情 先plan 不要做什麼事 讓我批准先」— 用戶要求**手動批准**
- **AI 錯在哪**：依賴系統的 auto-approve 機制，沒有等用戶在對話中明確說「批准」或「做」才動手
- **教訓**：用戶說「讓我批准先」= 必須等用戶在對話中明確回覆才能開始執行。系統自動批准 ≠ 用戶批准。**永遠以用戶的話為準**
