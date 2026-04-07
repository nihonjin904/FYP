# 主角 VRM Model 換皮任務

**⚠️ 所有代碼改動只對 Player 生效，不影響 BP_SekiroEnemy**

---

## 🔴 問題 C：人物面向反了（面向鏡頭，應該背對鏡頭）

### 修正方法：Blueprint 裡改 VRMMesh Rotation

C++ 的 rotation 代碼已經全部移除，不會干擾。

**步驟（明天做）：**

1. 打開 `BP_SekiroCharacter`
2. Components 面板 → 展開 `Mesh (CharacterMesh0)` → 點選 **VRMMesh**
3. 右邊 Details 面板 → **Transform** 區域 → **Rotation**
4. 你會看到三個輸入框，上面分別標著 **X** / **Y** / **Z**
5. 把 **Z** 那格改成 **180**（X 和 Y 保持 0）
6. 最終值 = **0 / 0 / 180**
7. Compile → Save

⚠️ 之前 bullshit #22 記錄了改錯 Y（第二格）的問題。**Y = Pitch**（會翻倒），**Z = Yaw**（水平旋轉）

**編譯 C++（因為移除了 rotation 代碼）：**
1. 關閉 UE5
2. Visual Studio → Ctrl+Shift+B
3. 重開 UE5

**預期結果：**
- Reimu 背對鏡頭，面向前方
- WASD 移動方向正確
- 鎖定時面向敵人

---

## ✅ 問題 B：武器掛載 — 已解決
## 🟡 問題 A：站姿後傾 — 等問題 C 解決後再調

---

### bullshit 記錄
lint errors（clang 報 undeclared identifier 等）是 Antigravity 的 clang 找不到 UE5 header，不影響 VS Build
