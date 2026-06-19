#include "UI/SekiroMainMenuWidget.h"
#include "SekiroGameInstance.h"
#include "SlateBasics.h"
#include "Widgets/Images/SImage.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

// ===== 色彩 =====
namespace MenuColors
{
	const FLinearColor Background  (0.01f, 0.01f, 0.03f, 0.92f);
	const FLinearColor TitleColor  (0.95f, 0.85f, 0.55f, 1.0f);
	const FLinearColor SubtitleColor(0.7f, 0.7f, 0.7f, 1.0f);
	const FLinearColor StartBg     (0.15f, 0.45f, 0.20f, 1.0f);
	const FLinearColor Selected    (0.85f, 0.65f, 0.15f, 1.0f);
	const FLinearColor Unselected  (0.25f, 0.25f, 0.30f, 1.0f);
	const FLinearColor EasyColor   (0.3f, 0.8f, 0.3f, 1.0f);
	const FLinearColor NormalColor (0.9f, 0.7f, 0.2f, 1.0f);
	const FLinearColor HardColor   (0.9f, 0.2f, 0.2f, 1.0f);
}

static FSlateFontInfo GetMenuFont(int32 Size, bool bBold = false)
{
	FSlateFontInfo FontInfo = FSlateFontInfo(
		FPaths::EngineContentDir() / (bBold ? TEXT("Slate/Fonts/Roboto-Bold.ttf") : TEXT("Slate/Fonts/Roboto-Regular.ttf")),
		Size);
	FontInfo.OutlineSettings.OutlineSize = Size >= 24 ? 2 : 1;
	FontInfo.OutlineSettings.OutlineColor = FLinearColor::Black;
	return FontInfo;
}

// ============================================================
// RebuildWidget — 逐段建
// ============================================================
TSharedRef<SWidget> USekiroMainMenuWidget::RebuildWidget()
{
	// 難度行
	TSharedRef<SWidget> EasyBtn = MakeDifficultyButton(TEXT("EASY"), TEXT("Relaxed combat"),
		EasyBorder, FOnClicked::CreateUObject(this, &USekiroMainMenuWidget::OnEasyClicked));
	TSharedRef<SWidget> NormalBtn = MakeDifficultyButton(TEXT("NORMAL"), TEXT("Balanced challenge"),
		NormalBorder, FOnClicked::CreateUObject(this, &USekiroMainMenuWidget::OnNormalClicked));
	TSharedRef<SWidget> HardBtn = MakeDifficultyButton(TEXT("HARD"), TEXT("True Shinobi"),
		HardBorder, FOnClicked::CreateUObject(this, &USekiroMainMenuWidget::OnHardClicked));

	TSharedRef<SWidget> StartBtn = MakeMenuButton(TEXT("START / CONTINUE"), 28, MenuColors::StartBg,
		FOnClicked::CreateUObject(this, &USekiroMainMenuWidget::OnStartClicked));

	TSharedRef<SWidget> RestartBtn = MakeMenuButton(TEXT("RESTART GAME"), 28, MenuColors::HardColor,
		FOnClicked::CreateUObject(this, &USekiroMainMenuWidget::OnRestartClicked));

	// 主佈局
	TSharedRef<SVerticalBox> Layout = SNew(SVerticalBox);

	// Title
	Layout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 8)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("ARCANE SOULS")))
		.Font(GetMenuFont(64, true))
		.ColorAndOpacity(MenuColors::TitleColor)
	];

	// Subtitle
	Layout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 50)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Rebirth")))
		.Font(GetMenuFont(26))
		.ColorAndOpacity(MenuColors::SubtitleColor)
	];

	// Difficulty label
	Layout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 15)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("SELECT DIFFICULTY")))
		.Font(GetMenuFont(18, true))
		.ColorAndOpacity(MenuColors::SubtitleColor)
	];

	// Difficulty buttons row
	TSharedRef<SHorizontalBox> DiffRow = SNew(SHorizontalBox);
	DiffRow->AddSlot().AutoWidth().Padding(8, 0)[EasyBtn];
	DiffRow->AddSlot().AutoWidth().Padding(8, 0)[NormalBtn];
	DiffRow->AddSlot().AutoWidth().Padding(8, 0)[HardBtn];

	Layout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 15)
	[
		DiffRow
	];

	// Selected difficulty text
	Layout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 40)
	[
		SAssignNew(DifficultyLabel, STextBlock)
		.Text(FText::FromString(TEXT("NORMAL")))
		.Font(GetMenuFont(20, true))
		.ColorAndOpacity(MenuColors::NormalColor)
	];

	// START button
	Layout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 20)
	[
		StartBtn
	];

	Layout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 20)
	[
		RestartBtn
	];

	// ===== SETTINGS: BGM Volume Slider =====
	// 讀取 GameInstance 已存音量
	float InitVolume = 0.7f;
	if (USekiroGameInstance* GI = Cast<USekiroGameInstance>(GetGameInstance()))
		InitVolume = GI->BGMVolume;

	Layout->AddSlot().AutoHeight().HAlign(HAlign_Left).Padding(0, 0, 0, 4)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("♪  BGM VOLUME")))
		.Font(GetMenuFont(14, true))
		.ColorAndOpacity(MenuColors::SubtitleColor)
	];

	Layout->AddSlot().AutoHeight().HAlign(HAlign_Fill).Padding(0, 0, 0, 16)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
		[
			SAssignNew(BGMVolumeSlider, SSlider)
			.Value(InitVolume)
			.OnValueChanged(FOnFloatValueChanged::CreateUObject(this, &USekiroMainMenuWidget::OnBGMVolumeChanged))
			.SliderBarColor(FLinearColor(0.4f, 0.3f, 0.1f, 0.8f))
			.SliderHandleColor(MenuColors::Selected)
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10, 0, 0, 0)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText {
				if (BGMVolumeSlider.IsValid())
					return FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(BGMVolumeSlider->GetValue() * 100)));
				return FText::FromString(TEXT("70%"));
			})
			.Font(GetMenuFont(14, true))
			.ColorAndOpacity(MenuColors::TitleColor)
		]
	];
	// ===== END BGM SETTINGS =====

	// ===== SETTINGS: SFX Volume Slider =====
	float InitSFXVolume = 0.5f;
	if (USekiroGameInstance* GI2 = Cast<USekiroGameInstance>(GetGameInstance()))
		InitSFXVolume = GI2->SFXVolume;

	Layout->AddSlot().AutoHeight().HAlign(HAlign_Left).Padding(0, 0, 0, 4)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("⚔  SFX VOLUME")))
		.Font(GetMenuFont(14, true))
		.ColorAndOpacity(MenuColors::SubtitleColor)
	];

	Layout->AddSlot().AutoHeight().HAlign(HAlign_Fill).Padding(0, 0, 0, 16)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
		[
			SAssignNew(SFXVolumeSlider, SSlider)
			.Value(InitSFXVolume)
			.OnValueChanged(FOnFloatValueChanged::CreateUObject(this, &USekiroMainMenuWidget::OnSFXVolumeChanged))
			.SliderBarColor(FLinearColor(0.4f, 0.1f, 0.1f, 0.8f))
			.SliderHandleColor(MenuColors::HardColor)
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10, 0, 0, 0)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText {
				if (SFXVolumeSlider.IsValid())
					return FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(SFXVolumeSlider->GetValue() * 100)));
				return FText::FromString(TEXT("50%"));
			})
			.Font(GetMenuFont(14, true))
			.ColorAndOpacity(MenuColors::TitleColor)
		]
	];
	// ===== END SFX SETTINGS =====

	// Version
	Layout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 30, 0, 0)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("FYP Demo Build v1.0")))
		.Font(GetMenuFont(12))
		.ColorAndOpacity(FLinearColor(0.4f, 0.4f, 0.4f, 0.6f))
	];
	// ===== 全屏佈局 — 背景圖 + 多層疊加 =====

	// 載入背景圖
	UTexture2D* BgTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/Textures/T_MainMenu_BG_Presentation"));
	TSharedRef<SWidget> BgWidget = SNullWidget::NullWidget;
	if (BgTexture)
	{
		BackgroundBrush = MakeShareable(new FSlateDynamicImageBrush(
			BgTexture,
			FVector2D(1920, 1080),
			FName(*BgTexture->GetPathName())
		));
		BgWidget = SNew(SImage).Image(BackgroundBrush.Get());
	}

	return SNew(SOverlay)

	// Layer 0: 背景圖（全屏拉伸）
	+ SOverlay::Slot()
	[
		BgWidget
	]



	// Layer 2: 內容面板（透明背景，保留佈局）
	+ SOverlay::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SBox)
		.WidthOverride(700)
		.HeightOverride(650)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.f))
			.Padding(FMargin(3))
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.f))
				.Padding(FMargin(1))
				[
					SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.f))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Padding(FMargin(30, 40))
					[
						Layout
					]
				]
			]
		]
	];
}

// ============================================================
void USekiroMainMenuWidget::NativeDestruct()
{
	BackgroundBrush.Reset();
	Super::NativeDestruct();
}

// ============================================================
void USekiroMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateDifficultyVisuals();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeUIOnly());
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
}

// ============================================================
FReply USekiroMainMenuWidget::OnEasyClicked()
{
	SelectedDifficulty = ESekirodifficulty::Easy;
	UpdateDifficultyVisuals();
	return FReply::Handled();
}

FReply USekiroMainMenuWidget::OnNormalClicked()
{
	SelectedDifficulty = ESekirodifficulty::Normal;
	UpdateDifficultyVisuals();
	return FReply::Handled();
}

FReply USekiroMainMenuWidget::OnHardClicked()
{
	SelectedDifficulty = ESekirodifficulty::Hard;
	UpdateDifficultyVisuals();
	return FReply::Handled();
}

FReply USekiroMainMenuWidget::OnStartClicked()
{
	if (USekiroGameInstance* GI = Cast<USekiroGameInstance>(GetGameInstance()))
	{
		GI->SetDifficulty(SelectedDifficulty);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	RemoveFromParent();

	return FReply::Handled();
}

FReply USekiroMainMenuWidget::OnRestartClicked()
{
	if (USekiroGameInstance* GI = Cast<USekiroGameInstance>(GetGameInstance()))
	{
		GI->SetDifficulty(SelectedDifficulty);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
	
	return FReply::Handled();
}

// ============================================================
void USekiroMainMenuWidget::UpdateDifficultyVisuals()
{
	auto SetBorder = [](TSharedPtr<SBorder>& Border, bool bSelected)
	{
		if (Border.IsValid())
		{
			Border->SetBorderBackgroundColor(bSelected ? MenuColors::Selected : MenuColors::Unselected);
		}
	};

	SetBorder(EasyBorder,   SelectedDifficulty == ESekirodifficulty::Easy);
	SetBorder(NormalBorder, SelectedDifficulty == ESekirodifficulty::Normal);
	SetBorder(HardBorder,   SelectedDifficulty == ESekirodifficulty::Hard);

	if (DifficultyLabel.IsValid())
	{
		switch (SelectedDifficulty)
		{
		case ESekirodifficulty::Easy:
			DifficultyLabel->SetText(FText::FromString(TEXT("EASY")));
			DifficultyLabel->SetColorAndOpacity(MenuColors::EasyColor);
			break;
		case ESekirodifficulty::Normal:
			DifficultyLabel->SetText(FText::FromString(TEXT("NORMAL")));
			DifficultyLabel->SetColorAndOpacity(MenuColors::NormalColor);
			break;
		case ESekirodifficulty::Hard:
			DifficultyLabel->SetText(FText::FromString(TEXT("HARD")));
			DifficultyLabel->SetColorAndOpacity(MenuColors::HardColor);
			break;
		}
	}
}

// ============================================================
TSharedRef<SWidget> USekiroMainMenuWidget::MakeDifficultyButton(
	const FString& Label,
	const FString& Desc,
	TSharedPtr<SBorder>& OutBorder,
	FOnClicked ClickDelegate)
{
	TSharedRef<SVerticalBox> BtnContent = SNew(SVerticalBox);
	BtnContent->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 8, 0, 2)
	[
		SNew(STextBlock)
		.Text(FText::FromString(Label))
		.Font(GetMenuFont(22, true))
		.ColorAndOpacity(FLinearColor::White)
	];
	BtnContent->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 5)
	[
		SNew(STextBlock)
		.Text(FText::FromString(Desc))
		.Font(GetMenuFont(11))
		.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 0.8f))
	];

	return SNew(SBox)
		.WidthOverride(190)
		.HeightOverride(90)
		[
			SAssignNew(OutBorder, SBorder)
			.BorderBackgroundColor(MenuColors::Unselected)
			.Padding(FMargin(3))
			[
				SNew(SButton)
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.OnClicked(ClickDelegate)
				[
					BtnContent
				]
			]
		];
}

// ============================================================
TSharedRef<SWidget> USekiroMainMenuWidget::MakeMenuButton(
	const FString& Label,
	float FontSize,
	FLinearColor BgColor,
	FOnClicked ClickDelegate)
{
	return SNew(SBox)
		.WidthOverride(380)
		.HeightOverride(65)
		[
			SNew(SBorder)
			.BorderBackgroundColor(BgColor)
			.Padding(FMargin(2))
			[
				SNew(SButton)
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.OnClicked(ClickDelegate)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Label))
					.Font(GetMenuFont(FontSize, true))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
		];
}

// ============================================================
void USekiroMainMenuWidget::OnBGMVolumeChanged(float NewValue)
{
	if (USekiroGameInstance* GI = Cast<USekiroGameInstance>(GetGameInstance()))
	{
		GI->SetBGMVolume(NewValue);
	}
}

void USekiroMainMenuWidget::OnSFXVolumeChanged(float NewValue)
{
	if (USekiroGameInstance* GI = Cast<USekiroGameInstance>(GetGameInstance()))
	{
		GI->SetSFXVolume(NewValue);
	}
}
