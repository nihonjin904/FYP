#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Components/AudioComponent.h"
#include "SekiroGameInstance.generated.h"

UENUM(BlueprintType)
enum class ESekirodifficulty : uint8
{
	Easy    UMETA(DisplayName = "Easy"),
	Normal  UMETA(DisplayName = "Normal"),
	Hard    UMETA(DisplayName = "Hard")
};

/**
 * 儲存跨 Level 設定（例如難度）
 */
UCLASS()
class FYP_API USekiroGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	/** 目前選擇的難度，預設 Normal */
	UPROPERTY(BlueprintReadWrite, Category="Sekiro|Difficulty")
	ESekirodifficulty CurrentDifficulty = ESekirodifficulty::Normal;

	/** Blueprint 呼叫：設定難度 */
	UFUNCTION(BlueprintCallable, Category="Sekiro|Difficulty")
	void SetDifficulty(ESekirodifficulty NewDifficulty);

	/** Blueprint 呼叫：獲取難度 */
	UFUNCTION(BlueprintPure, Category="Sekiro|Difficulty")
	ESekirodifficulty GetDifficulty() const { return CurrentDifficulty; }

	/** 便捷函數：開始遊戲（載入戰鬥關卡） */
	UFUNCTION(BlueprintCallable, Category="Sekiro|Difficulty")
	void StartGameWithDifficulty(ESekirodifficulty Difficulty, FName LevelName);

	// ===== BGM 音量 =====
	/** BGM 音量 (0.0 - 1.0)，跨 Level 保持 */
	UPROPERTY(BlueprintReadWrite, Category="Sekiro|Audio")
	float BGMVolume = 0.7f;

	/** 設定 BGM 音量並即時套用 */
	UFUNCTION(BlueprintCallable, Category="Sekiro|Audio")
	void SetBGMVolume(float NewVolume);

	// ===== SFX 音量 =====
	/** SFX 音量 (0.0 - 1.0)，預設 50% */
	UPROPERTY(BlueprintReadWrite, Category="Sekiro|Audio")
	float SFXVolume = 0.25f;

	/** 設定 SFX 音量 */
	UFUNCTION(BlueprintCallable, Category="Sekiro|Audio")
	void SetSFXVolume(float NewVolume);

	// ===== BGM 播放系統 =====
	/** 當前播放中嘅 BGM AudioComponent */
	UPROPERTY()
	UAudioComponent* BGMAudioComponent = nullptr;

	/** BGM Sound 資源路徑 */
	UPROPERTY()
	USoundBase* BGMSoundAsset = nullptr;

	/** 每次切換 World 時重新播放 BGM */
	void PlayBGM();
};
