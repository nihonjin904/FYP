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

