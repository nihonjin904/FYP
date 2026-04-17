#include "UI/SekiroGameHUDWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
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
			Brush.ImageSize = FVector2D(300.f, 300.f);
			MinimapImage->SetBrush(Brush);
		}
	}

	// ===== Minimap: Build checkpoint dots =====
	BuildCheckpointDots();
}

void USekiroGameHUDWidget::NativeDestruct()
{
	if (Instance == this)
		Instance = nullptr;
	CheckpointDots.Empty();
	Super::NativeDestruct();
}

// ===================================================
// Minimap Checkpoint Dots
// ===================================================

void USekiroGameHUDWidget::BuildCheckpointDots()
{
	if (!MinimapDotsPanel || !WidgetTree)
		return;

	CheckpointDots.Empty();
	MinimapDotsPanel->ClearChildren();

	// Find all checkpoint actors in the level
	TArray<AActor*> Checkpoints;
	UGameplayStatics::GetAllActorsOfClassWithTag(
		GetWorld(), AActor::StaticClass(), FName("Checkpoint"), Checkpoints);

	// Fallback: find by name pattern
	if (Checkpoints.Num() == 0)
	{
		TArray<AActor*> All;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), All);
		for (AActor* A : All)
		{
			if (A && A->GetName().Contains(TEXT("Checkpoint")))
				Checkpoints.Add(A);
		}
	}

	const float DotSize  = 14.f;
	const float HalfPanel = MinimapDisplaySize * 0.5f;

	// Build a reusable circular brush (RoundedBox with full corner radius = circle)
	FSlateBrush CircleBrush;
	CircleBrush.DrawAs    = ESlateBrushDrawType::RoundedBox;
	CircleBrush.ImageSize = FVector2D(DotSize, DotSize);
	CircleBrush.TintColor = FSlateColor(FLinearColor::White); // fill color via ColorAndOpacity
	CircleBrush.OutlineSettings.CornerRadii    = FVector4(DotSize * 0.5f, DotSize * 0.5f, DotSize * 0.5f, DotSize * 0.5f);
	CircleBrush.OutlineSettings.RoundingType   = ESlateBrushRoundingType::FixedRadius;
	CircleBrush.OutlineSettings.Color          = FSlateColor(FLinearColor::Black);
	CircleBrush.OutlineSettings.Width          = 2.f;

	for (AActor* Actor : Checkpoints)
	{
		if (!Actor) continue;

		// Create an Image dot widget
		UImage* Dot = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		if (!Dot) continue;

		// Apply circular brush + initial gray fill
		Dot->SetBrush(CircleBrush);
		Dot->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.f));

		// Add to canvas panel
		UCanvasPanelSlot* CanvasSlot = MinimapDotsPanel->AddChildToCanvas(Dot);
		if (CanvasSlot)
		{
			CanvasSlot->SetSize(FVector2D(DotSize, DotSize));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f)); // position targets dot center
			CanvasSlot->SetPosition(FVector2D(HalfPanel, HalfPanel));
		}

		FMinimapDotInfo Info;
		Info.ActorName = Actor->GetName();
		Info.WorldPos  = Actor->GetActorLocation();
		Info.DotWidget = Dot;
		CheckpointDots.Add(Info);
	}
}

void USekiroGameHUDWidget::RefreshCheckpointDots(ASekiroCharacter* Player)
{
	if (!Player || CheckpointDots.Num() == 0)
		return;

	const FVector  PlayerLoc  = Player->GetActorLocation();
	const float    PlayerYaw  = Player->GetActorRotation().Yaw;
	const float    HalfPanel  = MinimapDisplaySize * 0.5f;
	const float    Scale      = MinimapDisplaySize / (MinimapOrthoRadius * 2.f);
	const float    YawRad     = FMath::DegreesToRadians(-PlayerYaw);
	const float    CosYaw     = FMath::Cos(YawRad);
	const float    SinYaw     = FMath::Sin(YawRad);

	const float Radius    = MinimapDisplaySize * 0.5f; // minimap circle radius in pixels
	const float DotMargin = 8.f;                        // hide dot if center is this close to/beyond edge

	for (FMinimapDotInfo& Info : CheckpointDots)
	{
		if (!Info.DotWidget) continue;

		// World offset (2D)
		const float DX = Info.WorldPos.X - PlayerLoc.X;
		const float DY = Info.WorldPos.Y - PlayerLoc.Y;

		// Rotate into player-local space (player faces up)
		const float LocalFwd   =  DX * CosYaw - DY * SinYaw;
		const float LocalRight =  DX * SinYaw + DY * CosYaw;

		// Convert to minimap pixel coords
		const float PixelX = HalfPanel + LocalRight * Scale;
		const float PixelY = HalfPanel - LocalFwd   * Scale;

		// Hide dot if outside the circular minimap boundary
		const float DistFromCenter = FMath::Sqrt(
			FMath::Square(PixelX - HalfPanel) + FMath::Square(PixelY - HalfPanel));
		if (DistFromCenter > Radius - DotMargin)
		{
			Info.DotWidget->SetVisibility(ESlateVisibility::Hidden);
			continue;
		}
		Info.DotWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

		// Update position
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Info.DotWidget->Slot))
			CanvasSlot->SetPosition(FVector2D(PixelX, PixelY));

		// Color: green = activated, gray = inactive
		const bool bActive = Player->ActivatedCheckpointNames.Contains(Info.ActorName);
		Info.DotWidget->SetColorAndOpacity(
			bActive
				? FLinearColor(0.f, 1.f, 0.2f, 1.f)
				: FLinearColor(0.5f, 0.5f, 0.5f, 1.f));
	}
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
		PlayerArrow->SetRenderTransformAngle(0.f);
	}

	// ===== Minimap: Refresh checkpoint dots every 0.5s =====
	MinimapDotTimer += InDeltaTime;
	if (MinimapDotTimer >= MinimapDotRefreshInterval)
	{
		MinimapDotTimer = 0.f;
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (ASekiroCharacter* SekiroChar = Cast<ASekiroCharacter>(PC->GetPawn()))
				RefreshCheckpointDots(SekiroChar);
		}
	}
}
