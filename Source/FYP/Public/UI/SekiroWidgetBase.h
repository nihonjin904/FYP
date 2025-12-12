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
	UFUNCTION(BlueprintImplementableEvent, Category="Sekiro|UI")
	void UpdatePlayerPosture(float Current, float Max);

	UFUNCTION(BlueprintImplementableEvent, Category="Sekiro|UI")
	void UpdateEnemyPosture(float Current, float Max);
};
