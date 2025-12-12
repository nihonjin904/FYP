#include "Components/SekiroEnemyAttributeComponent.h"
#include "Components/SekiroCombatComponent.h"
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
			CombatComp->RequestAttack();
			TimeSinceLastAttack = 0.0f;
		}
	}
}
