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

	// TODO: Check if IncomingAttackType is unblockable (Perilous Attack)
	// If unblockable, return Failed unless specific counter logic is implemented.

	OnParryResult.Broadcast(Result);
	return Result;
}
