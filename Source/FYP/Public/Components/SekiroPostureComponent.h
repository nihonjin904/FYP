#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SekiroPostureComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPostureChanged, float, CurrentPosture, float, MaxPosture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPostureBroken);

UCLASS( ClassGroup=(Sekiro), meta=(BlueprintSpawnableComponent) )
class FYP_API USekiroPostureComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USekiroPostureComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core Posture Logic
	UFUNCTION(BlueprintCallable, Category="Sekiro|Posture")
	void AddPostureDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category="Sekiro|Posture")
	void ResetPosture();

	UFUNCTION(BlueprintCallable, Category="Sekiro|Posture")
	float GetPostureRatio() const;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Posture")
	float MaxPosture = 200.0f;  // 100→2001: 玩家可以据挻更多次

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Posture")
	float PostureRegenRateIdle = 12.0f;  // 5→12: 戰鬥間隙快速回復

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Posture")
	float PostureRegenRateBlocking = 25.0f;  // 15→25: 持對密檔加快回復

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Posture")
	float PostureRegenDelayAfterDamage = 3.0f;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sekiro|Posture")
	float CurrentPosture = 0.0f;

	UPROPERTY(BlueprintAssignable, Category="Sekiro|Posture")
	FOnPostureChanged OnPostureChanged;

	UPROPERTY(BlueprintAssignable, Category="Sekiro|Posture")
	FOnPostureBroken OnPostureBroken;

protected:
	float LastDamageTime = 0.0f;

	// Helper to check if owner is holding block
	bool IsOwnerBlocking() const;
};
