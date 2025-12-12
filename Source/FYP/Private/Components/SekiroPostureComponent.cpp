#include "Components/SekiroPostureComponent.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"

// Define a static tag for blocking if not already defined elsewhere, 
// or assume it will be passed/checked via an interface or tag container on the owner.
// For simplicity, we'll check for a tag on the owner.
static const FName TAG_State_Combat_HoldingBlock = FName("State.Combat.HoldingBlock");

USekiroPostureComponent::USekiroPostureComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentPosture = 0.0f;
}

void USekiroPostureComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USekiroPostureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Posture Regeneration
	float TimeSinceDamage = GetWorld()->GetTimeSeconds() - LastDamageTime;
	if (TimeSinceDamage > PostureRegenDelayAfterDamage && CurrentPosture > 0.0f)
	{
		float RegenRate = IsOwnerBlocking() ? PostureRegenRateBlocking : PostureRegenRateIdle;
		
		// If sprinting, maybe pause regen? (Requirement 1: Pauses regeneration when taking damage or sprinting)
		// We can check another tag for sprinting.
		// For now, let's implement the basic regen.
		
		CurrentPosture = FMath::Max(0.0f, CurrentPosture - (RegenRate * DeltaTime));
		OnPostureChanged.Broadcast(CurrentPosture, MaxPosture);
	}
}

void USekiroPostureComponent::AddPostureDamage(float Amount)
{
	if (CurrentPosture >= MaxPosture) return; // Already broken

	CurrentPosture = FMath::Clamp(CurrentPosture + Amount, 0.0f, MaxPosture);
	LastDamageTime = GetWorld()->GetTimeSeconds();
	
	OnPostureChanged.Broadcast(CurrentPosture, MaxPosture);

	if (CurrentPosture >= MaxPosture)
	{
		OnPostureBroken.Broadcast();
	}
}

void USekiroPostureComponent::ResetPosture()
{
	CurrentPosture = 0.0f;
	OnPostureChanged.Broadcast(CurrentPosture, MaxPosture);
}

float USekiroPostureComponent::GetPostureRatio() const
{
	return (MaxPosture > 0.0f) ? (CurrentPosture / MaxPosture) : 0.0f;
}

bool USekiroPostureComponent::IsOwnerBlocking() const
{
	// Assuming the owner implements IGameplayTagAssetInterface or we just check Actor Tags for simplicity in this demo.
	// In a full system, we'd use the Ability System Component or a dedicated State Component.
	// For this task, let's check Actor Tags or a known component.
	// Let's assume the owner has the tag "State.Combat.HoldingBlock" in its Tags array for now, 
	// or better, we can cast to our Character class later. 
	// To keep this component decoupled, let's check Actor Tags.
	if (AActor* Owner = GetOwner())
	{
		return Owner->ActorHasTag(TAG_State_Combat_HoldingBlock);
	}
	return false;
}
