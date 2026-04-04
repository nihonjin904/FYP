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

