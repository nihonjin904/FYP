#include "Components/SekiroCombatComponent.h"
#include "Components/SekiroPostureComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MotionWarpingComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Components/SekiroDeflectComponent.h"

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
	// 1. Visual Debug
	FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Action.Attack.Light"));
	OnAttackPerformed.Broadcast(AttackTag);
	
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Print Debug
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("%s Attacking!"), *Owner->GetName()));

	// 2. Physical Lunge (Simulate Animation Root Motion)
	ACharacter* Character = Cast<ACharacter>(Owner);
	if (Character)
	{
		FVector LaunchDir = Owner->GetActorForwardVector();
		Character->LaunchCharacter(LaunchDir * 500.0f, true, false);
	}

	// 3. Hit Detection (Delayed slightly to match "impact" - simplified here to immediate)
	FVector Start = Owner->GetActorLocation();
	FVector End = Start + (Owner->GetActorForwardVector() * AttackRange);
	
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult, 
		Start, 
		End, 
		FQuat::Identity, 
		ECC_Pawn, 
		FCollisionShape::MakeSphere(50.0f), 
		QueryParams
	);

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		
		// Check for Deflect Component
		USekiroDeflectComponent* DeflectComp = HitActor->FindComponentByClass<USekiroDeflectComponent>();
		USekiroPostureComponent* PostureComp = HitActor->FindComponentByClass<USekiroPostureComponent>();

		if (DeflectComp)
		{
			// Try to Parry
			EParryResult Result = DeflectComp->TryParry(AttackPostureDamage);
			
			if (Result == EParryResult::Perfect)
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("PERFECT PARRY!"));
				// TODO: Add posture damage to attacker (Self)
			}
			else if (Result == EParryResult::Blocked)
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Blocked!"));
				// Add reduced posture damage to defender
				if (PostureComp) PostureComp->AddPostureDamage(AttackPostureDamage * 0.5f);
			}
			else // Failed
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Hit!"));
				// Add full posture damage
				if (PostureComp) PostureComp->AddPostureDamage(AttackPostureDamage);
			}
		}
		else if (PostureComp)
		{
			// No deflect component, just take damage
			PostureComp->AddPostureDamage(AttackPostureDamage);
		}
	}
	
	// Draw Debug Line
	DrawDebugSphere(GetWorld(), End, 50.0f, 12, FColor::Red, false, 1.0f);
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
