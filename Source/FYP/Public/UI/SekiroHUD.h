#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SekiroHUD.generated.h"

UCLASS()
class FYP_API ASekiroHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Sekiro|UI")
	TSubclassOf<class USekiroWidgetBase> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category="Sekiro|UI")
	TObjectPtr<class USekiroWidgetBase> HUDWidget;
};
