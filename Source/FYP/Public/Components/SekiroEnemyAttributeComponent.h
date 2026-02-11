#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "SekiroEnemyAttributeComponent.generated.h"

class USekiroCombatComponent;

UCLASS( ClassGroup=(Sekiro), meta=(BlueprintSpawnableComponent) )
class FYP_API USekiroEnemyAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USekiroEnemyAttributeComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Simple AI Logic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy")
	bool bAutoAttack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy")
	float AttackInterval = 3.0f;

	/** Number of attacks per combo cycle (e.g. 4 for Combo_Attack_04). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy", meta=(ClampMin="1"))
	int32 ComboAttackCount = 4;

	/** Delay in seconds between each Attack() call in a combo (should align with combo window, e.g. 0.45s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy", meta=(ClampMin="0.1"))
	float ComboAttackInterval = 0.45f;

protected:
	float TimeSinceLastAttack = 0.0f;

	USekiroCombatComponent* CombatComp = nullptr;

	FTimerHandle ComboTimerHandle;
	int32 ComboAttacksRemaining = 0;

	void StartComboAttackCycle();
	void OnComboAttackTimer();
};
