#include "Components/SekiroCombatComponent.h"
#include "Components/SekiroPostureComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MotionWarpingComponent.h"

USekiroCombatComponent::USekiroCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Default tag for stunned state
	ContainerTag_Stunned = FGameplayTag::RequestGameplayTag(FName("State.Stunned"));
}

void USekiroCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USekiroCombatComponent::RequestAttack()
{
	// Logic to determine combo chain, stamina check, etc.
	// For now, we just broadcast basic attack.
	// In a real system, this would play a Montage.
	
	FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Action.Attack.Light"));
	OnAttackPerformed.Broadcast(AttackTag);
}

bool USekiroCombatComponent::TryExecuteTarget(AActor* TargetActor)
{
	if (!CanExecuteTarget(TargetActor))
	{
		return false;
	}

	// Trigger Execution
	OnExecutionTriggered.Broadcast(TargetActor);

	// Here we would typically:
	// 1. Play Execution Montage on Self (with Motion Warping to Target)
	// 2. Play Death Montage on Target
	// 3. Reset Target Posture or Kill Target

	// Example Motion Warping setup (Conceptual):
	// UMotionWarpingComponent* MotionWarping = GetOwner()->FindComponentByClass<UMotionWarpingComponent>();
	// if (MotionWarping)
	// {
	//     MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation("ExecutionTarget", TargetActor->GetActorLocation(), TargetActor->GetActorRotation());
	// }

	return true;
}

bool USekiroCombatComponent::CanExecuteTarget(AActor* TargetActor) const
{
	if (!TargetActor) return false;

	// Check distance
	float Distance = GetOwner()->GetDistanceTo(TargetActor);
	if (Distance > ExecutionRange) return false;

	// Check if target is stunned
	// Assuming target uses GameplayTags or has a specific component state
	// For this demo, we check if they have the "State.Stunned" tag
	if (!TargetActor->ActorHasTag(ContainerTag_Stunned.GetTagName()))
	{
		// Alternatively, check Posture Component directly
		USekiroPostureComponent* TargetPosture = GetTargetPosture(TargetActor);
		if (TargetPosture)
		{
			// If posture is broken (>= Max), they are vulnerable
			if (TargetPosture->CurrentPosture < TargetPosture->MaxPosture)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

USekiroPostureComponent* USekiroCombatComponent::GetTargetPosture(AActor* Target) const
{
	if (!Target) return nullptr;
	return Target->FindComponentByClass<USekiroPostureComponent>();
}
