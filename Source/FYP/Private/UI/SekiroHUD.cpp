#include "UI/SekiroHUD.h"
#include "UI/SekiroWidgetBase.h"
#include "Blueprint/UserWidget.h"
#include "Characters/SekiroCharacter.h"
#include "Components/SekiroAttributeComponent.h"
#include "Components/SekiroPostureComponent.h"
#include "Components/SekiroEnemyAttributeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

// ===== Constructor: 自動載入 WBP_PerilousWarning Widget Class =====
ASekiroHUD::ASekiroHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> WarningWidgetFinder(
		TEXT("/Game/UI/WBP_PerilousWarning"));
	if (WarningWidgetFinder.Succeeded())
	{
		PerilousWarningWidgetClass = WarningWidgetFinder.Class;
	}
}

void ASekiroHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<USekiroWidgetBase>(GetWorld(), HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();

			// === 綁定玩家的血量和架勢條 ===
			APawn* PlayerPawn = GetOwningPawn();
			if (PlayerPawn)
			{
				USekiroAttributeComponent* PlayerAttr = PlayerPawn->FindComponentByClass<USekiroAttributeComponent>();
				if (PlayerAttr)
				{
					PlayerAttr->OnHealthChanged.AddDynamic(this, &ASekiroHUD::OnPlayerHealthChanged);
					// 初始化顯示
					HUDWidget->UpdateHealth(PlayerAttr->CurrentHealth, PlayerAttr->MaxHealth);
				}

				USekiroPostureComponent* PlayerPosture = PlayerPawn->FindComponentByClass<USekiroPostureComponent>();
				if (PlayerPosture)
				{
					PlayerPosture->OnPostureChanged.AddDynamic(this, &ASekiroHUD::OnPlayerPostureChanged);
					HUDWidget->UpdatePlayerPosture(PlayerPosture->CurrentPosture, PlayerPosture->MaxPosture);
				}
			}

			// === 綁定敵人的血量和架勢條 ===
			// 查找帶有 "Enemy" Tag 的 Actor
			for (TActorIterator<AActor> It(GetWorld()); It; ++It)
			{
				AActor* Actor = *It;
				if (Actor && Actor->ActorHasTag(FName("Enemy")))
				{
					USekiroAttributeComponent* EnemyAttr = Actor->FindComponentByClass<USekiroAttributeComponent>();
					if (EnemyAttr)
					{
						EnemyAttr->OnHealthChanged.AddDynamic(this, &ASekiroHUD::OnEnemyHealthChanged);
						HUDWidget->UpdateEnemyHealth(EnemyAttr->CurrentHealth, EnemyAttr->MaxHealth);
					}

					USekiroPostureComponent* EnemyPosture = Actor->FindComponentByClass<USekiroPostureComponent>();
					if (EnemyPosture)
					{
						EnemyPosture->OnPostureChanged.AddDynamic(this, &ASekiroHUD::OnEnemyPostureChanged);
						HUDWidget->UpdateEnemyPosture(EnemyPosture->CurrentPosture, EnemyPosture->MaxPosture);
					}

					// ===== 綁定「危」Perilous Attack 事件 =====
					USekiroEnemyAttributeComponent* EnemyAI = Actor->FindComponentByClass<USekiroEnemyAttributeComponent>();
					if (EnemyAI)
					{
						EnemyAI->OnPerilousAttackStarted.AddDynamic(this, &ASekiroHUD::OnPerilousAttackStarted);
					}

					break; // 只綁定第一個找到的敵人
				}
			}
		}
	}
}

// ===== 「危」字繪製 =====

void ASekiroHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bShowPerilousWarning)
	{
		PerilousWarningTimer -= DeltaSeconds;
		if (PerilousWarningTimer <= 0.f)
		{
			bShowPerilousWarning = false;
			PerilousWarningTimer = 0.f;
		}
	}
}

void ASekiroHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!bShowPerilousWarning) return;

	// 取得螢幕尺寸
	const float ScreenW = Canvas->SizeX;
	const float ScreenH = Canvas->SizeY;

	// ===== 畫紅色「危」字 =====
	// 位置：螢幕中央偏上（30% 高度位置）
	const float PosX = ScreenW * 0.5f;
	const float PosY = ScreenH * 0.30f;

	// 閃爍效果：前 0.3 秒快速閃爍
	float Alpha = 1.0f;
	float TimeElapsed = PerilousWarningDuration - PerilousWarningTimer;
	if (TimeElapsed < 0.3f)
	{
		// 快速閃爍（每 0.1 秒切換）
		Alpha = (FMath::Fmod(TimeElapsed, 0.1f) < 0.05f) ? 1.0f : 0.3f;
	}
	else if (PerilousWarningTimer < 0.5f)
	{
		// 最後 0.5 秒漸漸消失
		Alpha = PerilousWarningTimer / 0.5f;
	}

	// 使用 Engine 默認大字體
	UFont* BigFont = GEngine->GetLargeFont();

	// 紅色背景光暈（多層疊加）
	FLinearColor GlowColor = FLinearColor(0.8f, 0.0f, 0.0f, Alpha * 0.4f);
	for (int32 dx = -2; dx <= 2; ++dx)
	{
		for (int32 dy = -2; dy <= 2; ++dy)
		{
			if (dx == 0 && dy == 0) continue;
			DrawText(TEXT("\x5371"), // 「危」的 Unicode
				GlowColor,
				PosX - 30.f + dx * 2.f, PosY - 30.f + dy * 2.f,
				BigFont, 4.0f);
		}
	}

	// 主文字：亮紅色
	FLinearColor MainColor = FLinearColor(1.0f, 0.1f, 0.1f, Alpha);
	DrawText(TEXT("\x5371"), // 「危」
		MainColor,
		PosX - 30.f, PosY - 30.f,
		BigFont, 4.0f);
}

void ASekiroHUD::OnPerilousAttackStarted()
{
	// ===== 顯示 WBP_PerilousWarning (「避」字) Widget =====
	if (PerilousWarningWidgetClass && GetOwningPlayerController())
	{
		// 首次使用時建立 Widget
		if (!PerilousWarningWidgetInstance)
		{
			PerilousWarningWidgetInstance = CreateWidget<UUserWidget>(
				GetOwningPlayerController(), PerilousWarningWidgetClass);
		}

		// 加入視口（如未加入）
		if (PerilousWarningWidgetInstance && !PerilousWarningWidgetInstance->IsInViewport())
		{
			PerilousWarningWidgetInstance->AddToViewport(10);
		}

		// 計時自動隱藏
		FTimerHandle HideHandle;
		GetWorldTimerManager().SetTimer(HideHandle, [this]()
		{
			if (PerilousWarningWidgetInstance && PerilousWarningWidgetInstance->IsInViewport())
			{
				PerilousWarningWidgetInstance->RemoveFromViewport();
			}
		}, PerilousWarningDuration, false);
	}

	// ===== Canvas 「危」字（備用） =====
	bShowPerilousWarning = true;
	PerilousWarningTimer = PerilousWarningDuration;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red,
			TEXT("=== PERILOUS ATTACK WARNING TRIGGERED ==="));
	}
}

// ===== 原有回調 =====

void ASekiroHUD::OnPlayerHealthChanged(float NewHealth, float MaxHealth)
{
	if (HUDWidget)
		HUDWidget->UpdateHealth(NewHealth, MaxHealth);
}

void ASekiroHUD::OnPlayerPostureChanged(float CurrentPosture, float MaxPosture)
{
	if (HUDWidget)
		HUDWidget->UpdatePlayerPosture(CurrentPosture, MaxPosture);
}

void ASekiroHUD::OnEnemyHealthChanged(float NewHealth, float MaxHealth)
{
	if (HUDWidget)
		HUDWidget->UpdateEnemyHealth(NewHealth, MaxHealth);
}

void ASekiroHUD::OnEnemyPostureChanged(float CurrentPosture, float MaxPosture)
{
	if (HUDWidget)
		HUDWidget->UpdateEnemyPosture(CurrentPosture, MaxPosture);
}
