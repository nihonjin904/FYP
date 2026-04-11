#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SekiroWidgetBase.generated.h"

UCLASS()
class FYP_API USekiroWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Event called when Posture changes, implemented in Blueprint
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Sekiro|UI")
	void UpdatePlayerPosture(float Current, float Max);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Sekiro|UI")
	void UpdateEnemyPosture(float Current, float Max);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Sekiro|UI")
	void UpdateHealth(float Current, float Max);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Sekiro|UI")
	void UpdateEnemyHealth(float Current, float Max);

protected:
	virtual void NativeConstruct() override;

private:
	/** 自動綁定：血量變化 → 呼叫 UpdateHealth */
	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float MaxHealth);

	/** 自動綁定：架勢變化 → 呼叫 UpdateEnemyPosture */
	UFUNCTION()
	void HandlePostureChanged(float CurrentPosture, float MaxPosture);
};
