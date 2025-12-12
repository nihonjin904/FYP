#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Simple AI Logic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy")
	bool bAutoAttack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Enemy")
	float AttackInterval = 3.0f;

protected:
	float TimeSinceLastAttack = 0.0f;
	
	USekiroCombatComponent* CombatComp;
};
