#include "Components/SekiroEnemyAttributeComponent.h"
#include "Components/SekiroCombatComponent.h"
#include "Characters/SekiroCharacter.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SekiroDeflectComponent.h"
#include "Components/SekiroAttributeComponent.h"
#include "Components/SekiroPostureComponent.h"
#include "Sound/SoundBase.h"
#include "Blueprint/UserWidget.h" // 「避」字 Widget
#include "SekiroGameInstance.h"     // 難度系統

USekiroEnemyAttributeComponent::USekiroEnemyAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USekiroEnemyAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	CombatComp = GetOwner()->FindComponentByClass<USekiroCombatComponent>();

	// ===== 自動遷移策略：同步 SpecialMontages 至 PerilousAttackMontages (不清空原陣列) =====
	// 注意：SpecialMontages 必須保留，否則 SekiroCombatComponent::RequestAttack() 的
	//       SpecialMontages.Num() > 0 檢查會返回 false，危攻擊永遠不會播放
	if (PerilousAttackMontages.Num() == 0 && CombatComp && CombatComp->SpecialMontages.Num() > 0)
	{
		PerilousAttackMontages = CombatComp->SpecialMontages; // 同步到 Perilous 清單
		// ⚠️ 不清空 SpecialMontages！清空會導致 RequestAttack() 永遠走普通攻擊路徑

		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
				FString::Printf(TEXT("[PerilousAttack] ✅ Synced %d montages (SpecialMontages preserved)"),
					PerilousAttackMontages.Num()));
	}

	// Debug: 顯示當前 Perilous Montage 數量
	if (GEngine)
	{
		FString Msg = FString::Printf(TEXT("[PerilousAttack] Montages ready: %d"), PerilousAttackMontages.Num());
		GEngine->AddOnScreenDebugMessage(-1, 5.0f,
			PerilousAttackMontages.Num() > 0 ? FColor::Cyan : FColor::Red, Msg);
	}

	// ===== 自動載入危攻擊命中音效 =====
	if (!PerilousAttackHitSound)
	{
		PerilousAttackHitSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/sound_effect/處決聲音.處決聲音"));
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 3.f,
				PerilousAttackHitSound ? FColor::Green : FColor::Red,
				PerilousAttackHitSound ? TEXT("[PerilousAttack] ✅ 音效載入成功") : TEXT("[PerilousAttack] ❌ 音效載入失敗"));
	}

	// ===== 自動載入「避」字 Widget Class =====
	CachedPerilousWarningWidgetClass = LoadClass<UUserWidget>(
		nullptr, TEXT("/Game/UI/WBP_PerilousWarning.WBP_PerilousWarning_C"));
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.f,
			CachedPerilousWarningWidgetClass ? FColor::Green : FColor::Red,
			CachedPerilousWarningWidgetClass
				? TEXT("[PerilousWarning] ✅ WBP_PerilousWarning 載入成功")
				: TEXT("[PerilousWarning] ❌ WBP_PerilousWarning 載入失敗"));

	// ===== 監聽自身 delegate （任何人 Broadcast 都觸發顯示 Widget） =====
	OnPerilousAttackStarted.AddDynamic(this, &USekiroEnemyAttributeComponent::InternalOnPerilousAttackStarted);

	// ===== 難度系統：讀 GameInstance 調整 Boss 數值 =====
	if (USekiroGameInstance* GI = Cast<USekiroGameInstance>(GetWorld()->GetGameInstance()))
	{
		USekiroAttributeComponent* AttrComp = GetOwner()->FindComponentByClass<USekiroAttributeComponent>();
		USekiroCombatComponent*    Combat   = CombatComp; // 已在上面初始化

		switch (GI->GetDifficulty())
		{
		case ESekirodifficulty::Easy:
			// === Easy: Boss 血少、攻擊慢、危攻擊少 ===
			if (AttrComp) { AttrComp->MaxHealth *= 0.6f; AttrComp->CurrentHealth = AttrComp->MaxHealth; }
			AttackInterval         = 4.0f;   // 原 3.0s → 更慢
			ComboAttackCount       = 2;      // 原 4 → 更短 combo
			PerilousAttackChance   = 0.10f;  // 原 0.3 → 10% 危攻擊
			if (Combat) { Combat->AttackPostureDamage = 12.0f; Combat->PerilousPostureDamage = 18.0f; }
			break;

		case ESekirodifficulty::Normal:
			// === Normal: 保持預設值 ===
			break;

		case ESekirodifficulty::Hard:
			// === Hard: Boss 血多、攻擊快、危攻擊多 ===
			if (AttrComp) { AttrComp->MaxHealth *= 1.5f; AttrComp->CurrentHealth = AttrComp->MaxHealth; }
			AttackInterval         = 2.0f;   // 更快攻擊
			ComboAttackCount       = 6;      // 更長 combo
			PerilousAttackChance   = 0.55f;  // 55% 危攻擊
			ComboAttackInterval    = 0.35f;  // combo 間隔更短
			if (Combat) { Combat->AttackPostureDamage = 28.0f; Combat->PerilousPostureDamage = 40.0f; }
			break;
		}

		if (GEngine)
		{
			const FString DiffStr = (GI->GetDifficulty() == ESekirodifficulty::Easy)   ? TEXT("Easy") :
			                        (GI->GetDifficulty() == ESekirodifficulty::Hard)   ? TEXT("Hard") : TEXT("Normal");
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
				FString::Printf(TEXT("[Difficulty] Boss scaled to: %s"), *DiffStr));
		}
	}
}

void USekiroEnemyAttributeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ComboTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ComboTimerHandle);
		}
		ComboTimerHandle.Invalidate();
	}
	Super::EndPlay(EndPlayReason);
}

void USekiroEnemyAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 敵人面向玩家（打主角而唔係打一個方向）
	if (bFacePlayer)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		AActor* OwnerActor = GetOwner();
		if (PlayerPawn && OwnerActor && PlayerPawn != OwnerActor)
		{
			float DistSq = FVector::DistSquared(OwnerActor->GetActorLocation(), PlayerPawn->GetActorLocation());
			if (DistSq <= FacePlayerRange * FacePlayerRange)
			{
				FVector ToPlayer = (PlayerPawn->GetActorLocation() - OwnerActor->GetActorLocation()).GetSafeNormal2D();
				if (ToPlayer.IsNormalized())
				{
					FRotator DesiredYaw = ToPlayer.Rotation();
					FRotator Current = OwnerActor->GetActorRotation();
					float Speed = FacePlayerSpeed * DeltaTime;
					FRotator NewRot = FMath::RInterpTo(Current, DesiredYaw, DeltaTime, FacePlayerSpeed);
					OwnerActor->SetActorRotation(NewRot);
				}
			}
		}
	}

	// Debug: unconditional tick trace every 3s
	static float GlobalDebugTimer = 0.f;
	GlobalDebugTimer += DeltaTime;
	if (GlobalDebugTimer >= 3.0f) {
		GlobalDebugTimer = 0.f;
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::White,
			FString::Printf(TEXT("EnemyAI Tick: bAutoAttack=%d CombatComp=%s"),
				bAutoAttack, CombatComp ? TEXT("OK") : TEXT("NULL")));
	}

	if (bAutoAttack && CombatComp && !ComboTimerHandle.IsValid() && !bIsPerilousAttacking)
	{
		TimeSinceLastAttack += DeltaTime;
		// Debug: show status every 3 sec
		static float DebugTimer = 0.f;
		DebugTimer += DeltaTime;
		if (DebugTimer >= 3.0f) {
			DebugTimer = 0.f;
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Orange,
				FString::Printf(TEXT("EnemyAI: bAutoAttack=%d CombatComp=%s TimeSince=%.1f / %.1f"),
					bAutoAttack, CombatComp ? TEXT("OK") : TEXT("NULL"),
					TimeSinceLastAttack, AttackInterval));
		}
		if (TimeSinceLastAttack >= AttackInterval)
		{

			TimeSinceLastAttack = 0.0f;
			StartComboAttackCycle();
		}
	}
}

void USekiroEnemyAttributeComponent::StartComboAttackCycle()
{
	ASekiroCharacter* SekiroChar = Cast<ASekiroCharacter>(GetOwner());
	if (!SekiroChar)
	{
		return;
	}

	// ===== 隨機觸發「避」Perilous 攻擊 =====
	if (TryPerilousAttack())
	{
		return; // 已觸發 Perilous 攻擊，不走普通 Combo
	}

	// First attack immediately
	SekiroChar->Attack();

	const int32 NumFollowUpAttacks = FMath::Max(0, ComboAttackCount - 1);
	if (NumFollowUpAttacks <= 0)
	{
		return;
	}

	ComboAttacksRemaining = NumFollowUpAttacks;
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		ComboTimerHandle,
		this,
		&USekiroEnemyAttributeComponent::OnComboAttackTimer,
		ComboAttackInterval,
		true
	);
}

void USekiroEnemyAttributeComponent::OnComboAttackTimer()
{
	ASekiroCharacter* SekiroChar = Cast<ASekiroCharacter>(GetOwner());
	if (SekiroChar)
	{
		SekiroChar->Attack();
	}

	--ComboAttacksRemaining;
	if (ComboAttacksRemaining <= 0)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(ComboTimerHandle);
		}
		ComboTimerHandle.Invalidate();
	}
}

// ===== 「避」Perilous Attack 實現 =====

bool USekiroEnemyAttributeComponent::TryPerilousAttack()
{
	// 檢查是否有 Perilous Montage 可用
	if (PerilousAttackMontages.Num() <= 0)
		return false;

	// 機率判定
	if (FMath::FRand() >= PerilousAttackChance)
		return false;

	ASekiroCharacter* SekiroChar = Cast<ASekiroCharacter>(GetOwner());
	if (!SekiroChar) return false;

	USkeletalMeshComponent* Mesh = SekiroChar->GetMesh();
	if (!Mesh) return false;

	UAnimInstance* AnimInst = Mesh->GetAnimInstance();
	if (!AnimInst) return false;

	// 隨機選一個 Perilous Montage（Sweep 或 Thrust）
	int32 Idx = FMath::RandRange(0, PerilousAttackMontages.Num() - 1);
	UAnimMontage* PerilMontage = PerilousAttackMontages[Idx];
	if (!PerilMontage) return false;

	// ===== 提前 0.8s 廣播「危/避」字 → 玩家有更多時間反應 =====
	// 先廣播，0.8s 後才播動畫，玩家有 ~1.6s 反應窗口
	OnPerilousAttackStarted.Broadcast();
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
		TEXT("⚠ BOSS: PERILOUS ATTACK!"));

	bIsPerilousAttacking = true;

	// 0.8s 後播放動畫 + 設置命中判定
	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(DelayHandle, [this, PerilMontage, AnimInst]()
	{
		if (!AnimInst || !PerilMontage) return;

		float Duration = AnimInst->Montage_Play(PerilMontage, 1.0f);
		if (Duration <= 0.0f) { bIsPerilousAttacking = false; return; }

		// 綁定 Montage 結束回調
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &USekiroEnemyAttributeComponent::OnPerilousAttackMontageEnded);
		AnimInst->Montage_SetEndDelegate(EndDelegate, PerilMontage);

		// 在動畫中段執行攻擊判定（用 Attack.Perilous Tag）
		float HitTime = Duration * 0.4f;
		GetWorld()->GetTimerManager().SetTimer(
			ComboTimerHandle,
			[this]() {
				AActor* Owner = GetOwner();
				if (!Owner) return;

				static const FGameplayTag PerilousTag = 
					FGameplayTag::RequestGameplayTag(FName("Attack.Perilous"), false);

				FVector Start = Owner->GetActorLocation();
				FVector End = Start + (Owner->GetActorForwardVector() * 250.0f);
				FHitResult HitResult;
				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(Owner);

				bool bHit = GetWorld()->SweepSingleByChannel(
					HitResult, Start, End, FQuat::Identity, ECC_Pawn,
					FCollisionShape::MakeSphere(80.0f), QueryParams);

				if (bHit && HitResult.GetActor())
				{
					AActor* HitActor = HitResult.GetActor();
					USekiroDeflectComponent* DeflectComp = 
						HitActor->FindComponentByClass<USekiroDeflectComponent>();
					USekiroAttributeComponent* AttrComp =
						HitActor->FindComponentByClass<USekiroAttributeComponent>();
					USekiroPostureComponent* PostureComp =
						HitActor->FindComponentByClass<USekiroPostureComponent>();

					if (DeflectComp)
					{
						EParryResult Result = DeflectComp->TryParry(PerilousTag);
						if (Result == EParryResult::Failed)
						{
							if (AttrComp)
								AttrComp->ApplyDamage(10.0f * PerilousAttackDamageMultiplier);
							if (PostureComp)
								PostureComp->AddPostureDamage(0.0f);
							if (PerilousAttackHitSound) {
								float SFXVol = 0.25f;
								if (USekiroGameInstance* GI = Cast<USekiroGameInstance>(GetWorld()->GetGameInstance()))
									SFXVol = GI->SFXVolume;
								UGameplayStatics::PlaySoundAtLocation(GetWorld(),
									PerilousAttackHitSound, Owner->GetActorLocation(), SFXVol);
							}
							if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
								TEXT("[PerilousAttack] ⚠ PERILOUS HIT! Player damaged!"));
						}
					}
					else
					{
						if (AttrComp)
							AttrComp->ApplyDamage(10.0f * PerilousAttackDamageMultiplier);
					}
				}
				ComboTimerHandle.Invalidate();
			},
			HitTime, false);

	}, 0.8f, false);

	return true;
}

void USekiroEnemyAttributeComponent::OnPerilousAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsPerilousAttacking = false;

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow,
		FString::Printf(TEXT("Perilous Attack Ended (interrupted=%d)"), bInterrupted));
}

// ===== 「避」字 Widget 直接顯示（繞過 HUD 依賴） =====

void USekiroEnemyAttributeComponent::InternalOnPerilousAttackStarted()
{
	ShowPerilousWarningWidget();
}

void USekiroEnemyAttributeComponent::ShowPerilousWarningWidget()
{
	// 取得 PlayerController
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("[PerilousWarning] ❌ No PlayerController"));
		return;
	}

	if (!CachedPerilousWarningWidgetClass)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("[PerilousWarning] ❌ Widget Class is null"));
		return;
	}

	// 建立 Widget（只建一次，重用）
	if (!PerilousWarningWidgetInstance)
	{
		PerilousWarningWidgetInstance = CreateWidget<UUserWidget>(PC, CachedPerilousWarningWidgetClass);
	}

	if (!PerilousWarningWidgetInstance)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("[PerilousWarning] ❌ CreateWidget failed"));
		return;
	}

	// 加入 Viewport（ZOrder=10 確保在最前）
	if (!PerilousWarningWidgetInstance->IsInViewport())
	{
		PerilousWarningWidgetInstance->AddToViewport(10);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("[PerilousWarning] ✅ 「避」字顯示！"));
	}

	// 1.5 秒後自動隱藏
	FTimerHandle HideHandle;
	GetWorld()->GetTimerManager().SetTimer(HideHandle, [this]()
	{
		if (PerilousWarningWidgetInstance && PerilousWarningWidgetInstance->IsInViewport())
		{
			PerilousWarningWidgetInstance->RemoveFromParent();
		}
	}, 1.5f, false);
}
