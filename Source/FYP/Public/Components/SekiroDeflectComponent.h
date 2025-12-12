#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SekiroDeflectComponent.generated.h"

UENUM(BlueprintType)
enum class EParryResult : uint8
{
	Perfect UMETA(DisplayName = "Perfect Parry"),
	Blocked UMETA(DisplayName = "Blocked"),
	Failed  UMETA(DisplayName = "Failed")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParryResult, EParryResult, Result);

UCLASS( ClassGroup=(Sekiro), meta=(BlueprintSpawnableComponent) )
class FYP_API USekiroDeflectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USekiroDeflectComponent();

protected:
	virtual void BeginPlay() override;

public:	
	// Called when player presses Block button
	UFUNCTION(BlueprintCallable, Category="Sekiro|Combat")
	void StartBlocking();

	// Called when player releases Block button
	UFUNCTION(BlueprintCallable, Category="Sekiro|Combat")
	void StopBlocking();

	// Called when hit by an attack to determine result
	UFUNCTION(BlueprintCallable, Category="Sekiro|Combat")
	EParryResult TryParry(FGameplayTag IncomingAttackType);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sekiro|Combat")
	float PerfectParryWindow = 0.2f;

	UPROPERTY(BlueprintAssignable, Category="Sekiro|Combat")
	FOnParryResult OnParryResult;

protected:
	float BlockStartTime = -1.0f;
	bool bIsBlocking = false;
};
