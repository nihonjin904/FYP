#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SekiroCombatComponent.generated.h"

class USekiroPostureComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackPerformed, FGameplayTag, AttackType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExecutionTriggered, AActor*, Target);

UCLASS( ClassGroup=(Sekiro), meta=(BlueprintSpawnableComponent) )
class FYP_API USekiroCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USekiroCombatComponent();

protected:
	virtual void BeginPlay() override;

public:	
	// Attempt to perform an attack
	UFUNCTION(BlueprintCallable, Category="Sekiro|Combat")
	void RequestAttack();

protected:
	// Helper to get posture component from an actor
	USekiroPostureComponent* GetTargetPosture(AActor* Target) const;
};
