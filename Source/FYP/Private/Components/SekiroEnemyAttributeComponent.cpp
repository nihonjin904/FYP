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

USekiroEnemyAttributeComponent::USekiroEnemyAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USekiroEnemyAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	CombatComp = GetOwner()->FindComponentByClass<USekiroCombatComponent>();

	// Debug: 顯示當前 Perilous Montage 數量
	if (GEngine)
	{
		FString Msg = FString::Printf(TEXT("Perilous Montages loaded: %d"), PerilousAttackMontages.Num());
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, 
			PerilousAttackMontages.Num() > 0 ? FColor::Cyan : FColor::Yellow, Msg);
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

	// 播放 Perilous 攻擊動畫
	float Duration = AnimInst->Montage_Play(PerilMontage, 1.0f);
	if (Duration <= 0.0f) return false;

	bIsPerilousAttacking = true;

	// 廣播事件 → BP_SekiroEnemy 收到後顯示「避」字 UI
	OnPerilousAttackStarted.Broadcast();

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
		TEXT("⚠ BOSS: PERILOUS ATTACK!"));

	// 綁定 Montage 結束回調
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &USekiroEnemyAttributeComponent::OnPerilousAttackMontageEnded);
	AnimInst->Montage_SetEndDelegate(EndDelegate, PerilMontage);

	// 在動畫中段執行攻擊判定（用 Attack.Perilous Tag）
	float HitTime = Duration * 0.4f; // 動畫 40% 時觸發命中判定
	GetWorld()->GetTimerManager().SetTimer(
		ComboTimerHandle, // 複用 timer handle
		[this]() {
			// 執行 Perilous 攻擊命中判定
			AActor* Owner = GetOwner();
			if (!Owner) return;

			static const FGameplayTag PerilousTag = 
				FGameplayTag::RequestGameplayTag(FName("Attack.Perilous"), false);

			// Sphere trace 找目標
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
					// TryParry 會檢查 Attack.Perilous tag → 強制 Failed
					EParryResult Result = DeflectComp->TryParry(PerilousTag);
					if (Result == EParryResult::Failed)
					{
						// Perilous 攻擊命中：加倍傷害
						if (AttrComp)
							AttrComp->ApplyDamage(10.0f * PerilousAttackDamageMultiplier);
						if (PostureComp)
							PostureComp->AddPostureDamage(0.0f); // pause regen

						if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
							TEXT("PERILOUS HIT! Player damaged!"));
					}
				}
				else
				{
					// 沒有 DeflectComp → 直接扣血
					if (AttrComp)
						AttrComp->ApplyDamage(10.0f * PerilousAttackDamageMultiplier);
				}
			}

			ComboTimerHandle.Invalidate();
		},
		HitTime, false);

	return true;
}

void USekiroEnemyAttributeComponent::OnPerilousAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsPerilousAttacking = false;

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow,
		FString::Printf(TEXT("Perilous Attack Ended (interrupted=%d)"), bInterrupted));
}
