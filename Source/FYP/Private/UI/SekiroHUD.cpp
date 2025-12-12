#include "UI/SekiroHUD.h"
#include "UI/SekiroWidgetBase.h"
#include "Blueprint/UserWidget.h"

void ASekiroHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<USekiroWidgetBase>(GetWorld(), HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
}
