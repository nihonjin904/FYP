#include "Components/SekiroDeflectComponent.h"
#include "GameFramework/Actor.h"

USekiroDeflectComponent::USekiroDeflectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USekiroDeflectComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USekiroDeflectComponent::StartBlocking()
{
	bIsBlocking = true;
	BlockStartTime = GetWorld()->GetTimeSeconds();
	
	// Optional: Add Tag to Owner "State.Combat.HoldingBlock"
	// For now, we assume the Character class handles the Tag application 
	// or we can do it here if we cast Owner to a common interface.
	// To keep it simple, we just track time here.
}

void USekiroDeflectComponent::StopBlocking()
{
	bIsBlocking = false;
	BlockStartTime = -1.0f;
}

EParryResult USekiroDeflectComponent::TryParry(FGameplayTag IncomingAttackType)
{
	// ===== 「避」Perilous Attack：無法格擋，必須閃避/跳躍 =====
	static const FGameplayTag PerilousTag = FGameplayTag::RequestGameplayTag(FName("Attack.Perilous"), false);
	if (IncomingAttackType.MatchesTag(PerilousTag))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
			TEXT("⚠ PERILOUS ATTACK — CANNOT BLOCK!"));
		OnParryResult.Broadcast(EParryResult::Failed);
		return EParryResult::Failed;
	}
	// ===== 原有格擋邏輯 =====

	EParryResult Result = EParryResult::Blocked;

	if (bIsAI)
	{
		// AI Logic: Roll dice
		float Roll = FMath::FRand(); // 0.0 to 1.0

		if (Roll < DeflectProbability)
		{
			Result = EParryResult::Perfect;
		}
		else if (Roll < (DeflectProbability + BlockProbability))
		{
			Result = EParryResult::Blocked;
		}
		else
		{
			Result = EParryResult::Failed;
			OnParryResult.Broadcast(Result);
			return Result;
		}
	}
	else
	{
		// Player Logic: Check input timing
		if (!bIsBlocking)
		{
			OnParryResult.Broadcast(EParryResult::Failed);
			return EParryResult::Failed;
		}

		float CurrentTime = GetWorld()->GetTimeSeconds();
		float TimeSinceBlockStart = CurrentTime - BlockStartTime;

		if (TimeSinceBlockStart <= PerfectParryWindow)
		{
			Result = EParryResult::Perfect;
		}
		else
		{
			Result = EParryResult::Blocked;
		}
	}

	// (Perilous Attack 已在函數開頭處理 — Attack.Perilous tag → 強制 Failed)

	OnParryResult.Broadcast(Result);
	return Result;
}
