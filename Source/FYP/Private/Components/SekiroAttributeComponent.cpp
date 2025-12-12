#include "Components/SekiroAttributeComponent.h"

USekiroAttributeComponent::USekiroAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentHealth = MaxHealth;
}

void USekiroAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

void USekiroAttributeComponent::ApplyDamage(float DamageAmount)
{
	if (CurrentHealth <= 0.0f) return;

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		OnDeath.Broadcast();
	}
}

float USekiroAttributeComponent::GetHealthPercent() const
{
	return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}
