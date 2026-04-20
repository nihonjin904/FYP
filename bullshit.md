# Bullshit Log — AI 錯誤記錄

## [2026-04-18] UPROPERTY 改動後叫用 Live Coding → 必然 Crash

**錯誤描述：**
- 在 `SekiroEnemyAttributeComponent.h` 加入新的 `UPROPERTY()` 欄位後，叫用者用 `Ctrl+Alt+F11` Live Coding
- UE5 crash：`Fatal error: Cannot replace existing object of a different class`
- **原因**：UPROPERTY 影響 class memory layout → CDO 已在記憶體中 → Live Coding 無法 replace → 必須 Full Rebuild

**結論：任何 UPROPERTY / UFUNCTION 改動 = 禁止 Live Coding = 必須完整重新編譯**

**正確做法：**
1. 改完 .h / .cpp
2. 關 Editor
3. Visual Studio → Rebuild
4. 重開 Editor
