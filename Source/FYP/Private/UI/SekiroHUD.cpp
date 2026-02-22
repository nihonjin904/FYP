#include "UI/SekiroHUD.h"
#include "UI/SekiroWidgetBase.h"
#include "Blueprint/UserWidget.h"
#include "Characters/SekiroCharacter.h"
#include "Components/SekiroAttributeComponent.h"
#include "Components/SekiroPostureComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

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

					break; // 只綁定第一個找到的敵人
				}
			}
		}
	}
}

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
