#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SekiroAttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS( ClassGroup=(Sekiro), meta=(BlueprintSpawnableComponent) )
class FYP_API USekiroAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USekiroAttributeComponent();

protected:
	virtual void BeginPlay() override;

public:	
	UFUNCTION(BlueprintCallable, Category="Sekiro|Attributes")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category="Sekiro|Attributes")
	float GetHealthPercent() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Attributes")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sekiro|Attributes")
	float CurrentHealth;

	UPROPERTY(BlueprintAssignable, Category="Sekiro|Attributes")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="Sekiro|Attributes")
	FOnDeath OnDeath;
};
