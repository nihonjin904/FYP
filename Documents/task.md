# 主角 VRM Model 換皮任務

## 目標
將 `BP_SekiroCharacter` 的外觀從 UE5 Mannequin 替換為博麗靈夢（VRM），同時保留所有現有動畫（Idle、攻擊、擋刀、Perfect Parry、死亡）。

## 當前基線
- **Git 版本**: `0db895f` — Mannequin 完整版
- **Blueprint**: `BP_SekiroCharacter` 已手動加入 VRMMesh component

---

## 進度總覽

| 步驟 | 狀態 | 說明 |
|------|------|------|
| Step 0 | ✅ | Push baseline（commit `0db895f`） |
| Step 1 | ✅ | 確認 Editor 打開，BP 正常 |
| Step 2 | ✅ | Add SkeletalMeshComponent `VRMMesh`，attach to CharacterMesh0 |
| Step 3 | ✅ | CharacterMesh0 Hidden In Game = true |
| Step 4 | ✅ | Always Tick Pose and Refresh Bones |
| Step 5 | ✅ | Reimu 顯示成功，retarget 動畫在跑，走位方向正確 |
| Step 6 | 🔧 | 問題 A：人物斜著向上看（頭不看敵人） |
| Step 7 | 🔧 | 問題 B：武器（M_Katana）位置不對 |
| Step 8 | ⬜ | 確認敵人不受影響 |
| Step 9 | ⬜ | Push 完成版本 |

---

## 🔧 問題 A：人物斜著向上看

### 現象
- Rotation (0, 0, 0) 時走位方向是正確的（直走）
- 但角色身體/頭部有點斜斜向上看，頭不是看向前方/敵人
- VRMMesh 目前 Rotation = (0°, 0°, **5°**)

### 原因分析
這**不是** VRMMesh Component 的 Relative Rotation 問題（那個只影響整體朝向）。
真正原因是 **VRM 骨骼的 rest pose 跟 Mannequin 不同**：
- Mannequin 的 rest pose：頭正視前方，脊椎直立
- VRM 的 rest pose：頭可能微微仰起，脊椎角度不同
- `Retarget Pose From Mesh` 在映射時，這些差異會導致最終 pose 偏差

### 解決方案

**方案 1（推薦）：修正 VRMMesh 的 Roll**
- 先把 VRMMesh Rotation 改回 `(0, 0, 0)` 看看有沒有改善
- 目前 5° Roll 本身就會讓人物斜

**方案 2：調 Retarget Pose Asset**
- 打開 `POSE_retarget__魔_博麗_霊夢`（PoseAsset）
- 這個 asset 定義了 retarget 時的基準 pose
- 如果 Reimu 的 rest pose 跟 Mannequin 差太多，需要在這裡調整翻譯
- 這個操作**必須在 Editor 手動做**

**方案 3：調 IKRetargeter**
- 打開 `RTG__魔_博麗_霊夢`（IKRetargeter）
- 調整骨骼鏈的映射和 offset
- 這是最精準的方法但最複雜

### 建議操作順序
1. 先把 VRMMesh Roll 改回 0° → Compile → Play 測試
2. 如果還是歪，開 `RTG__魔_博麗_霊夢` 看 Preview，手動調 spine/head chain mapping

### 耗時：5-15 分鐘

---

## 🔧 問題 B：武器（M_Katana）位置不對

### 現象
武器（刀）的位置跟 Reimu 的手不吻合。

### 從截圖確認的 Component 結構
```
BP_SekiroCharacter (Self)
├── Mesh (CharacterMesh0) ← Mannequin，已 hidden
│   ├── Block Weapon Pivot (BlockWeaponPivot) ← attach 到 hand_r 骨骼
│   │   └── Weapon Mesh (WeaponMesh) ← M_Katana，Location=(-12, 3, 0), Rotation=(-41°, 11°, 42°)
│   └── VRMMesh ← 靈夢，Location=(-2, 0, 0), Rotation=(0, 0, 5°)
```

### 原因
WeaponMesh 掛在 **Mannequin** 的 `hand_r` 骨骼（通過 BlockWeaponPivot）。
Mannequin 被 Hidden 但骨骼仍更新，所以武器跟的是 **Mannequin 手的世界座標**。
但 Reimu 的手位置（retarget 後）跟 Mannequin 的手位置**不完全重合**（體型、arm length 不同），所以武器看起來偏了。

### 解決方案

#### 方案 B（推薦）：C++ runtime re-attach 武器到 VRMMesh 骨骼

**改動位置**：`SekiroCharacter.cpp` 的 `BeginPlay()` 函數末尾

**邏輯**：
```cpp
// 如果 BP 裡有 VRMMesh component，將武器 attach 過去
TArray<USkeletalMeshComponent*> SkelComps;
GetComponents<USkeletalMeshComponent>(SkelComps);
for (auto* Comp : SkelComps) {
    if (Comp != GetMesh() && Comp->GetSkeletalMeshAsset()) {
        // 找到 VRMMesh — 將 BlockWeaponPivot re-attach 到 VRM 的右手骨骼
        BlockWeaponPivot->AttachToComponent(
            Comp,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            FName("J_Bip_R_Hand")  // VRM 標準右手骨骼名
        );
        break;
    }
}
```

**需要確認**：Reimu 的右手骨骼名是否為 `J_Bip_R_Hand`。
- VRM4U 標準命名：`J_Bip_R_Hand` ✅
- 如果不對，可以在 Editor 打開 `SKEL__魔_博麗_霊夢` 查看骨骼名

**注意事項**：
- attach 後，WeaponMesh 的 Relative Transform 可能需要微調（因為 Reimu 的手骨 orientation 跟 Mannequin 的 `hand_r` 不同）
- 敵人（BP_SekiroEnemy）沒有 VRMMesh，所以 for loop 找不到就不會 re-attach，不影響敵人

**MCP 能否做**：❌（C++ 改動 + rebuild）

**耗時**：15-20 分鐘（寫代碼 5 分鐘 + rebuild 1 分鐘 + 調整 offset 10 分鐘）

---

## 下一步行動

1. **先修問題 A**：手動在 Editor 把 VRMMesh Roll 改回 0° → Play 測試
2. **再修問題 B**：確認後我寫 C++ re-attach 代碼 → rebuild → 調整 offset
3. **最後 Push**

---

## 錯誤記錄

1. ❌ 不要用 C++ `CreateDefaultSubobject` 加 VRM Component — Blueprint CDO 會衝突
2. ❌ 不要在 C++ 硬碼 ConstructorHelpers 載入 Reimu mesh — 敵人也會載入
3. ❌ 不要用 Live Coding 改 .h UPROPERTY — 必須完整 rebuild
4. ✅ 正確做法：Blueprint Editor Add Component，純配置，零代碼
5. ⚠️ VRM rest pose 跟 Mannequin 不同，retarget 後 head/spine 可能偏
6. ⚠️ 武器掛在 Mannequin hand_r，換皮後需 runtime re-attach 到 VRM 骨骼
