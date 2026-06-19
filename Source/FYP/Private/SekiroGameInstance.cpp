#include "SekiroGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "EngineUtils.h"

void USekiroGameInstance::Init()
{
	Super::Init();

	// 載入 BGM Sound 資源
	BGMSoundAsset = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/BGM_JapaneseShrineTheme"));
	if (!BGMSoundAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SekiroGameInstance] BGM asset not found at /Game/Audio/BGM_JapaneseShrineTheme"));
	}

	// 每次有新 World 被建立時自動播放 BGM
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddLambda([this](UWorld* LoadedWorld)
	{
		PlayBGM();
	});
}

void USekiroGameInstance::PlayBGM()
{
	if (!BGMSoundAsset) return;

	// 如果已經有播緊，唔需要重新播
	if (BGMAudioComponent && BGMAudioComponent->IsPlaying())
	{
		BGMAudioComponent->SetVolumeMultiplier(BGMVolume);
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	// 用 SpawnSound2D 播放非空間化音頻（全局 BGM）
	BGMAudioComponent = UGameplayStatics::SpawnSound2D(World, BGMSoundAsset, BGMVolume, 1.0f, 0.0f, nullptr, false, true);
	if (BGMAudioComponent)
	{
		BGMAudioComponent->bAutoDestroy = false;  // 唔好自動銷毀
		BGMAudioComponent->bIsUISound = true;      // UI sound = 唔受暫停影響
		BGMAudioComponent->Play();
		UE_LOG(LogTemp, Log, TEXT("[SekiroGameInstance] BGM started playing, volume=%.2f"), BGMVolume);
	}
}

void USekiroGameInstance::SetDifficulty(ESekirodifficulty NewDifficulty)
{
	CurrentDifficulty = NewDifficulty;
}

void USekiroGameInstance::StartGameWithDifficulty(ESekirodifficulty Difficulty, FName LevelName)
{
	CurrentDifficulty = Difficulty;
	UGameplayStatics::OpenLevel(this, LevelName);
}

void USekiroGameInstance::SetBGMVolume(float NewVolume)
{
	BGMVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);

	// 直接調整正在播放嘅 BGM AudioComponent
	if (BGMAudioComponent)
	{
		BGMAudioComponent->SetVolumeMultiplier(BGMVolume);
	}

	// 同時調整 Level 入面嘅 AmbientSound（如果有嘅話）
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetName().Contains(TEXT("BGM")))
			{
				if (UAudioComponent* AudioComp = Actor->FindComponentByClass<UAudioComponent>())
				{
					AudioComp->SetVolumeMultiplier(BGMVolume);
				}
			}
		}
	}
}

void USekiroGameInstance::SetSFXVolume(float NewVolume)
{
	SFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}
