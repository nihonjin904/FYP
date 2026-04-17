#include "UI/SekiroGameHUDWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/SekiroCharacter.h"

USekiroGameHUDWidget* USekiroGameHUDWidget::Instance = nullptr;

void USekiroGameHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Instance = this;

	// ===== Minimap: Create dynamic material instance and wire RenderTarget =====
	if (MinimapImage && MinimapMaskMaterial)
	{
		MinimapMID = UMaterialInstanceDynamic::Create(MinimapMaskMaterial, this);
		if (MinimapMID)
		{
			// Get player character to retrieve the RenderTarget
			if (APlayerController* PC = GetOwningPlayer())
			{
				if (ASekiroCharacter* SekiroChar = Cast<ASekiroCharacter>(PC->GetPawn()))
				{
					if (SekiroChar->MinimapRenderTarget)
					{
						MinimapMID->SetTextureParameterValue(TEXT("MinimapTexture"), SekiroChar->MinimapRenderTarget);
					}
				}
			}
			// Apply material to MinimapImage
			MinimapImage->SetBrushFromMaterial(MinimapMID);
			FSlateBrush Brush = MinimapImage->GetBrush();
			Brush.ImageSize = FVector2D(150.f, 150.f);
			MinimapImage->SetBrush(Brush);
		}
	}
}

void USekiroGameHUDWidget::NativeDestruct()
{
	if (Instance == this)
		Instance = nullptr;
	Super::NativeDestruct();
}

void USekiroGameHUDWidget::FlashWidgetByName(const FName& WidgetName)
{
	if (!WidgetTree)
		return;

	UWidget* FoundWidget = WidgetTree->FindWidget(WidgetName);
	if (!FoundWidget)
		return;

	// 檢查是否已在閃爍中，如果是則重置計時器
	for (FFlashInfo& Info : ActiveFlashes)
	{
		if (Info.Widget == FoundWidget)
		{
			Info.Timer = FlashDuration;
			FoundWidget->SetRenderOpacity(FlashMinOpacity);
			return;
		}
	}

	// 開始新的閃爍
	FFlashInfo NewFlash;
	NewFlash.Widget = FoundWidget;
	NewFlash.Timer = FlashDuration;
	ActiveFlashes.Add(NewFlash);

	FoundWidget->SetRenderOpacity(FlashMinOpacity);
}

void USekiroGameHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	for (int32 i = ActiveFlashes.Num() - 1; i >= 0; --i)
	{
		FFlashInfo& Info = ActiveFlashes[i];
		Info.Timer -= InDeltaTime;

		if (Info.Timer <= 0.0f)
		{
			// 閃爍結束，恢復正常
			if (Info.Widget)
				Info.Widget->SetRenderOpacity(1.0f);
			ActiveFlashes.RemoveAt(i);
		}
		else if (Info.Widget)
		{
			// 漸漸恢復不透明度（從 FlashMinOpacity → 1.0）
			float Alpha = 1.0f - (Info.Timer / FlashDuration);
			float Opacity = FMath::Lerp(FlashMinOpacity, 1.0f, Alpha);
			Info.Widget->SetRenderOpacity(Opacity);
		}
	}

	// ===== Minimap: PlayerArrow stays fixed (map rotates with player) =====
	if (PlayerArrow)
	{
		PlayerArrow->SetRenderTransformAngle(0.f); // Always points up — map rotation handles direction
	}
}
