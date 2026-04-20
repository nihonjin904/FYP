#include "UI/SekiroMapOverlayWidget.h"
#include "Characters/SekiroCharacter.h"
#include "Save/FYPSaveGame.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Blueprint/WidgetTree.h"

#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SizeBox.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// ============================================================
// NativeConstruct
// ============================================================

void USekiroMapOverlayWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Hide teleport prompt initially
    if (TeleportPrompt)
    {
        TeleportPrompt->SetVisibility(ESlateVisibility::Hidden);
    }

    // Wire RT_Minimap directly to MapImage via SlateBrush (no material needed)
    ASekiroCharacter* Player = Cast<ASekiroCharacter>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    if (Player && Player->MinimapRenderTarget && MapImage)
    {
        FSlateBrush Brush;
        Brush.SetResourceObject(Player->MinimapRenderTarget);
        Brush.ImageSize = FVector2D(1920.f, 1080.f);
        Brush.DrawAs = ESlateBrushDrawType::Image;
        MapImage->SetBrush(Brush);
    }

    // Ensure keyboard events reach this widget
    SetIsFocusable(true);

    // Icons need laid-out geometry — deferred to NativeTick
    bIconsBuilt = false;

    // Force keyboard focus so F / Escape work immediately
    SetKeyboardFocus();
}

// ============================================================
// NativeTick — build icons once geometry is available
// ============================================================

void USekiroMapOverlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bIconsBuilt)
    {
        FVector2D Size = MyGeometry.GetLocalSize();
        if (Size.X > 0.f)
        {
            BuildCheckpointIcons();
            bIconsBuilt = true;
        }
    }
}

// ============================================================
// BuildCheckpointIcons
// ============================================================

void USekiroMapOverlayWidget::BuildCheckpointIcons()
{
    // Use the root Canvas Panel from WidgetTree directly
    UCanvasPanel* RootPanel = WidgetTree
        ? Cast<UCanvasPanel>(WidgetTree->RootWidget)
        : nullptr;
    if (!RootPanel)
        return;

    // ── Clear previously-added dynamic icons ──────────────────
    for (TWeakObjectPtr<UWidget>& W : DynamicMapIcons)
        if (W.IsValid()) W->RemoveFromParent();
    DynamicMapIcons.Empty();
    DynamicIconSlots.Empty();

    CheckpointIcons.Empty();
    SelectedCheckpointIndex = -1;

    ASekiroCharacter* Player = Cast<ASekiroCharacter>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Player)
        return;

    // Use runtime activated list (already loaded in character BeginPlay)
    const TArray<FString>& ActivatedNames = Player->ActivatedCheckpointNames;

    // Widget pixel size
    FVector2D MapSize = GetCachedGeometry().GetLocalSize();
    if (MapSize.IsNearlyZero())
        MapSize = FVector2D(1920.f, 1080.f);

    // OrthoHeight is derived from the RT aspect ratio:
    // SceneCapture OrthoWidth covers horizontal; vertical = OrthoWidth * (RT_H / RT_W)
    float OrthoHeight = MapOrthoWidth; // default: square RT
    if (MapImage)
    {
        UTextureRenderTarget2D* RT = Cast<UTextureRenderTarget2D>(MapImage->Brush.GetResourceObject());
        if (RT && RT->SizeX > 0)
            OrthoHeight = MapOrthoWidth * (float)RT->SizeY / RT->SizeX;
    }

    // Camera position (panned by user, or player location initially)
    FVector CamPos = Player->MinimapCapture
        ? Player->MinimapCapture->GetComponentLocation()
        : Player->GetActorLocation();

    // ─── Player icon: correct ortho projection ────────────────
    {
        FVector PlayerLoc = Player->GetActorLocation();
        float PNormX = (PlayerLoc.Y - CamPos.Y) / MapOrthoWidth + 0.5f;
        float PNormY = 0.5f - (PlayerLoc.X - CamPos.X) / OrthoHeight;  // use OrthoHeight
        float PPixelX = PNormX * MapSize.X;
        float PPixelY = PNormY * MapSize.Y;

        UButton* PlayerDot = NewObject<UButton>(this);
        {
            FSlateBrush PBrush;
            PBrush.DrawAs    = ESlateBrushDrawType::RoundedBox;
            PBrush.ImageSize = FVector2D(16.f, 16.f);
            PBrush.TintColor = FSlateColor(FLinearColor(1.f, 0.85f, 0.f, 1.f));

            FSlateBrushOutlineSettings PO;
            PO.Width        = 2.f;
            PO.Color        = FLinearColor::Black;
            PO.RoundingType = ESlateBrushRoundingType::FixedRadius;
            PO.CornerRadii  = FVector4(8.f, 8.f, 8.f, 8.f);
            PBrush.OutlineSettings = PO;

            FButtonStyle PS;
            PS.Normal = PS.Hovered = PS.Pressed = PBrush;
            PlayerDot->SetStyle(PS);
            PlayerDot->SetIsEnabled(false);
        }
        UCanvasPanelSlot* PS = RootPanel->AddChildToCanvas(PlayerDot);
        PS->SetPosition(FVector2D(PPixelX - 8.f, PPixelY - 8.f));
        PS->SetSize(FVector2D(16.f, 16.f));
        PS->SetZOrder(20);
        DynamicMapIcons.Add(PlayerDot);
        DynamicIconSlots.Add(PS);
    }

    // ─── Find all checkpoint actors ────────────────────────────
    TArray<AActor*> Checkpoints;
    // Primary: by tag "Checkpoint"
    UGameplayStatics::GetAllActorsOfClassWithTag(
        GetWorld(), AActor::StaticClass(), FName("Checkpoint"), Checkpoints);
    // Fallback: by name pattern
    if (Checkpoints.Num() == 0)
    {
        TArray<AActor*> All;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), All);
        for (AActor* A : All)
            if (A && (A->GetName().Contains(TEXT("Checkpoint")) ||
                      A->GetName().Contains(TEXT("checkpoint"))))
                Checkpoints.Add(A);
    }

    for (AActor* Actor : Checkpoints)
    {
        if (!Actor) continue;

        FVector WP = Actor->GetActorLocation();

        // Ortho projection: world XY → normalised UV
        // X (horizontal) uses OrthoWidth; Y (vertical) uses OrthoHeight = OrthoWidth*(RT_H/RT_W)
        float NormX = (WP.Y - CamPos.Y) / MapOrthoWidth + 0.5f;
        float NormY = 0.5f - (WP.X - CamPos.X) / OrthoHeight;

        // Clamp visible icons only
        if (NormX < 0.f || NormX > 1.f || NormY < 0.f || NormY > 1.f)
            continue;

        float PixelX = NormX * MapSize.X;
        float PixelY = NormY * MapSize.Y;

        bool bActivated = ActivatedNames.Contains(Actor->GetName());

        FLinearColor DotColour = bActivated
            ? FLinearColor(0.f, 0.8f, 0.2f, 1.f)   // green
            : FLinearColor(0.45f, 0.45f, 0.45f, 1.f); // grey

        // ─── Speech bubble ───────────────────────────────────────
        UBorder* Bubble = NewObject<UBorder>(this);
        Bubble->SetBrushColor(FLinearColor(0.12f, 0.12f, 0.12f, 0.92f));
        Bubble->SetPadding(FMargin(6.f, 3.f, 6.f, 3.f));

        UTextBlock* Label = NewObject<UTextBlock>(this);
        Label->SetText(FText::FromString(TEXT("Checkpoint")));
        Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        {
            FSlateFontInfo Font = Label->GetFont();
            Font.Size = 10;
            Label->SetFont(Font);
        }
        Bubble->SetContent(Label);

        // ─── Dot button ──────────────────────────────────────────
        UButton* Dot = NewObject<UButton>(this);
        {
            FSlateBrush DotBrush;
            DotBrush.DrawAs    = ESlateBrushDrawType::RoundedBox;
            DotBrush.ImageSize = FVector2D(14.f, 14.f);
            DotBrush.TintColor = FSlateColor(DotColour);

            FSlateBrushOutlineSettings Outline;
            Outline.Width       = 2.f;
            Outline.Color       = FLinearColor::Black;
            Outline.RoundingType = ESlateBrushRoundingType::FixedRadius;
            Outline.CornerRadii = FVector4(7.f, 7.f, 7.f, 7.f);
            DotBrush.OutlineSettings = Outline;

            FButtonStyle DotStyle;
            DotStyle.Normal  = DotStyle.Hovered = DotStyle.Pressed = DotBrush;
            Dot->SetStyle(DotStyle);
        }

        if (bActivated)
            Dot->OnClicked.AddDynamic(this, &USekiroMapOverlayWidget::OnAnyCheckpointClicked);
        else
            Dot->SetIsEnabled(false);

        // ─── VBox: bubble above dot ──────────────────────────────
        UVerticalBox* VBox = NewObject<UVerticalBox>(this);
        UVerticalBoxSlot* BSlot = VBox->AddChildToVerticalBox(Bubble);
        BSlot->SetHorizontalAlignment(HAlign_Center);
        BSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 2.f));
        UVerticalBoxSlot* DSlot = VBox->AddChildToVerticalBox(Dot);
        DSlot->SetHorizontalAlignment(HAlign_Center);

        UCanvasPanelSlot* CPSlot = RootPanel->AddChildToCanvas(VBox);
        CPSlot->SetPosition(FVector2D(PixelX - 30.f, PixelY - 36.f));
        CPSlot->SetAutoSize(true);
        CPSlot->SetZOrder(10);
        DynamicMapIcons.Add(VBox);
        DynamicIconSlots.Add(CPSlot);

        FCheckpointIconInfo Info;
        Info.ActorName  = Actor->GetName();
        Info.WorldPos   = WP;
        Info.bActivated = bActivated;
        Info.Actor      = Actor;
        Info.DotButton  = Dot;
        CheckpointIcons.Add(Info);
    }
}

// ============================================================
// Click handler
// ============================================================

void USekiroMapOverlayWidget::OnAnyCheckpointClicked()
{
    // Find which DotButton sent this event by comparing against stored pointers.
    // UButton::OnClicked doesn't pass a sender, so we check which button
    // is currently "pressed" via IsPressed(), falling back to first activated.
    for (int32 i = 0; i < CheckpointIcons.Num(); i++)
    {
        if (CheckpointIcons[i].bActivated
            && CheckpointIcons[i].DotButton
            && CheckpointIcons[i].DotButton->IsPressed())
        {
            SelectCheckpoint(i);
            return;
        }
    }
    // Fallback: select first activated (handles edge cases)
    for (int32 i = 0; i < CheckpointIcons.Num(); i++)
    {
        if (CheckpointIcons[i].bActivated)
        {
            SelectCheckpoint(i);
            return;
        }
    }
}

void USekiroMapOverlayWidget::SelectCheckpoint(int32 Index)
{
    SelectedCheckpointIndex = Index;
    if (TeleportPrompt)
    {
        TeleportPrompt->SetText(FText::FromString(TEXT("[F] Teleport")));
        TeleportPrompt->SetVisibility(ESlateVisibility::Visible);
    }
    // Ensure F / Escape reach us
    SetKeyboardFocus();
}

// ============================================================
// Keyboard input
// ============================================================

FReply USekiroMapOverlayWidget::NativeOnKeyDown(
    const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    // Escape → close map
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        ASekiroCharacter* Player = Cast<ASekiroCharacter>(
            UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
        if (Player)
            Player->ToggleMap();
        return FReply::Handled();
    }

    // F → teleport to selected checkpoint
    if (InKeyEvent.GetKey() == EKeys::F
        && SelectedCheckpointIndex >= 0
        && SelectedCheckpointIndex < CheckpointIcons.Num())
    {
        const FCheckpointIconInfo& Icon = CheckpointIcons[SelectedCheckpointIndex];
        if (Icon.Actor.IsValid() && Icon.bActivated)
        {
            ASekiroCharacter* Player = Cast<ASekiroCharacter>(
                UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
            if (Player)
            {
                // Teleport: +100 Z to avoid floor clipping
                FVector TargetPos = Icon.Actor->GetActorLocation() + FVector(0.f, 0.f, 100.f);
                Player->SetActorLocation(
                    TargetPos, false, nullptr, ETeleportType::TeleportPhysics);

                // Zero out velocity to prevent momentum carry-over
                if (UCharacterMovementComponent* CMC = Player->GetCharacterMovement())
                {
                    CMC->Velocity = FVector::ZeroVector;
                    CMC->UpdateComponentVelocity();
                }

                // Close map (restores time dilation + game input)
                Player->ToggleMap();
            }
        }
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ============================================================
// Scroll wheel zoom
// ============================================================

FReply USekiroMapOverlayWidget::NativeOnMouseWheel(
    const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    MapZoom = FMath::Clamp(MapZoom + InMouseEvent.GetWheelDelta() * 0.15f, 0.5f, 3.0f);
    if (MapImage)
    {
        FWidgetTransform WT;
        WT.Scale = FVector2D(MapZoom, MapZoom);
        MapImage->SetRenderTransform(WT);
    }
    return FReply::Handled();
}

// ============================================================
// Mouse drag → pan the map capture
// ============================================================

FReply USekiroMapOverlayWidget::NativeOnMouseButtonDown(
    const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        bDragging = true;
        LastMousePos = InMouseEvent.GetScreenSpacePosition();
        return FReply::Handled().CaptureMouse(TakeWidget());
    }
    SelectedCheckpointIndex = -1;
    if (TeleportPrompt)
        TeleportPrompt->SetVisibility(ESlateVisibility::Hidden);
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply USekiroMapOverlayWidget::NativeOnMouseButtonUp(
    const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bDragging)
    {
        bDragging    = false;
        bIconsBuilt  = false; // rebuild from world positions now that drag finished
        return FReply::Handled().ReleaseMouseCapture();
    }
    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply USekiroMapOverlayWidget::NativeOnMouseMove(
    const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bDragging)
    {
        FVector2D CurrentPos = InMouseEvent.GetScreenSpacePosition();

        // Convert to local Slate units (same space as CanvasPanelSlot positions)
        FVector2D LocalCurrent = InGeometry.AbsoluteToLocal(CurrentPos);
        FVector2D LocalLast    = InGeometry.AbsoluteToLocal(LastMousePos);
        FVector2D LocalDelta   = LocalCurrent - LocalLast;
        LastMousePos = CurrentPos;

        FVector2D MapSize = InGeometry.GetLocalSize();
        if (!MapSize.IsNearlyZero())
        {
            // Move the SceneCapture in world space
            float WorldPerUnit = MapOrthoWidth / MapSize.X;
            float WorldDX =  LocalDelta.Y * WorldPerUnit;
            float WorldDY = -LocalDelta.X * WorldPerUnit;

            ASekiroCharacter* Player = Cast<ASekiroCharacter>(
                UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
            if (Player)
                Player->PanMapCapture(WorldDX, WorldDY);

            // Icon delta: same direction as LocalDelta (icons follow map content)
            // Y axis needs aspect correction: OrthoWidth/OrthoHeight * MapSize.Y/MapSize.X
            // = (RT_W/RT_H) * (MapSize.Y/MapSize.X)
            // For 16:9 RT on 16:9 display = 1.0 (no correction)
            // For square RT on 16:9 display = 1080/1920 ≈ 0.5625
            float OrthoHeightLocal = MapOrthoWidth;
            if (MapImage)
            {
                UTextureRenderTarget2D* RT2 = Cast<UTextureRenderTarget2D>(MapImage->Brush.GetResourceObject());
                if (RT2 && RT2->SizeX > 0)
                    OrthoHeightLocal = MapOrthoWidth * (float)RT2->SizeY / RT2->SizeX;
            }
            float YScale = (OrthoHeightLocal > 0.f)
                ? (MapOrthoWidth / OrthoHeightLocal) * (MapSize.Y / MapSize.X)
                : 1.0f;

            FVector2D IconDelta(LocalDelta.X, LocalDelta.Y * YScale);
            for (UCanvasPanelSlot* CPS : DynamicIconSlots)
            {
                if (!CPS) continue;
                CPS->SetPosition(CPS->GetPosition() + IconDelta);
            }
        }
        return FReply::Handled();
    }
    return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}
