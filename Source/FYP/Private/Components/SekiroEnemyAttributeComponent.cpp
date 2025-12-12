#include "Components/SekiroEnemyAttributeComponent.h"
#include "Components/SekiroCombatComponent.h"
#include "Characters/SekiroCharacter.h" // Added include
#include "GameFramework/Actor.h"

USekiroEnemyAttributeComponent::USekiroEnemyAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USekiroEnemyAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	CombatComp = GetOwner()->FindComponentByClass<USekiroCombatComponent>();
}

void USekiroEnemyAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAutoAttack && CombatComp)
	{
		TimeSinceLastAttack += DeltaTime;
		if (TimeSinceLastAttack >= AttackInterval)
		{
			// CombatComp->RequestAttack(); // Old logic (No animation)
			
			// New logic: Call Character's Attack function to play animation
			if (ASekiroCharacter* SekiroChar = Cast<ASekiroCharacter>(GetOwner()))
			{
				SekiroChar->Attack();
			}
			
			TimeSinceLastAttack = 0.0f;
		}
	}
}
