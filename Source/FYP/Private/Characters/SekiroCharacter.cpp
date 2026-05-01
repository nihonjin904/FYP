
#include "Characters/SekiroCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/Button.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SekiroAttributeComponent.h"
#include "Components/SekiroEnemyAttributeComponent.h"
#include "Components/SekiroCombatComponent.h"
#include "Components/SekiroDeflectComponent.h"
#include "Components/SekiroPostureComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/WorldSettings.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/SekiroGameHUDWidget.h"
#include "UI/SekiroWidgetBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

// Static constant for save slot name
const FString ASekiroCharacter::CheckpointSaveSlot = TEXT("FYP_Slot_0");


ASekiroCharacter::ASekiroCharacter() {
  PrimaryActorTick.bCanEverTick = true;

  // Don't rotate when the controller rotates. Let that just affect the camera.
  bUseControllerRotationPitch = false;
  bUseControllerRotationYaw = false;
  bUseControllerRotationRoll = false;

  // Configure character movement
  GetCharacterMovement()->bOrientRotationToMovement = true;
  GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

  // Create Camera Boom
  CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
  CameraBoom->SetupAttachment(RootComponent);
  CameraBoom->TargetArmLength = 400.0f;
  CameraBoom->bUsePawnControlRotation = true;

  // Create Follow Camera
  FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
  FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
  FollowCamera->bUsePawnControlRotation = false;

  // Create Combat Components
  PostureComponent =
      CreateDefaultSubobject<USekiroPostureComponent>(TEXT("PostureComponent"));
  DeflectComponent =
      CreateDefaultSubobject<USekiroDeflectComponent>(TEXT("DeflectComponent"));
  CombatComponent =
      CreateDefaultSubobject<USekiroCombatComponent>(TEXT("CombatComponent"));
  AttributeComponent = CreateDefaultSubobject<USekiroAttributeComponent>(
      TEXT("AttributeComponent"));

  // Create Overhead Widget
  OverheadWidget =
      CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
  OverheadWidget->SetupAttachment(RootComponent);
  OverheadWidget->SetWidgetSpace(EWidgetSpace::World);
  OverheadWidget->SetRelativeScale3D(FVector(0.05f, 0.05f, 0.05f));
  OverheadWidget->SetDrawAtDesiredSize(true);
  OverheadWidget->SetRelativeLocation(
      FVector(0.0f, 0.0f, 100.0f)); // Above head


  // Create Deathblow Widget
  DeathblowWidget =
      CreateDefaultSubobject<UWidgetComponent>(TEXT("DeathblowWidget"));
  DeathblowWidget->SetupAttachment(RootComponent);
  DeathblowWidget->SetWidgetSpace(EWidgetSpace::World);
  DeathblowWidget->SetRelativeScale3D(FVector(0.05f, 0.05f, 0.05f));
  DeathblowWidget->SetDrawAtDesiredSize(true);
  DeathblowWidget->SetVisibility(false); // Hidden by default
  DeathblowWidget->SetRelativeLocation(
      FVector(0.0f, 0.0f, 50.0f)); // Chest/Head level

  // BlockWeaponPivot：手 → Pivot → WeaponMesh，擋刀時只轉 Pivot
  // 就唔會被動畫蓋過
  // 注意：Constructor 用 WeaponSocketName 預設值；BP 覆寫值喺 BeginPlay 重新 Attach
  BlockWeaponPivot =
      CreateDefaultSubobject<USceneComponent>(TEXT("BlockWeaponPivot"));
  BlockWeaponPivot->SetupAttachment(GetMesh(), WeaponSocketName);

  WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
  WeaponMesh->SetupAttachment(
      BlockWeaponPivot); // 掛喺 Pivot 下面，唔直接掛 hand
  WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(
      TEXT("/Engine/BasicShapes/Cube.Cube"));
  if (CubeMeshAsset.Succeeded()) {
    WeaponMesh->SetStaticMesh(CubeMeshAsset.Object);
    WeaponMesh->SetWorldScale3D(FVector(0.1f, 0.1f, 1.0f));
  }

  // ================================================================
  // === 自動載入資源（不再需要手動在 Blueprint 設定）===
  // ================================================================

  // --- 音效 ---
  static ConstructorHelpers::FObjectFinder<USoundBase> PerfectParrySoundAsset(
      TEXT("/Game/sound_effect/Perfect_Parry_音效.Perfect_Parry_音效"));
  if (PerfectParrySoundAsset.Succeeded())
    PerfectParrySound = PerfectParrySoundAsset.Object;

  static ConstructorHelpers::FObjectFinder<USoundBase> BlockSoundAsset(
      TEXT("/Game/sound_effect/普通擋刀.普通擋刀"));
  if (BlockSoundAsset.Succeeded())
    BlockSound = BlockSoundAsset.Object;

  static ConstructorHelpers::FObjectFinder<USoundBase> ExecutionSoundAsset(
      TEXT("/Game/sound_effect/處決聲音.處決聲音"));
  if (ExecutionSoundAsset.Succeeded())
    ExecutionSound = ExecutionSoundAsset.Object;

  // --- Camera Shake ---
  static ConstructorHelpers::FClassFinder<UCameraShakeBase>
      PerfectParryShakeAsset(
          TEXT("/Game/character_block_particle/BP_PerfectParryShake"));
  if (PerfectParryShakeAsset.Succeeded())
    PerfectParryCameraShake = PerfectParryShakeAsset.Class;

  static ConstructorHelpers::FClassFinder<UCameraShakeBase> BlockShakeAsset(
      TEXT("/Game/character_block_particle/BP_BlockShake"));
  if (BlockShakeAsset.Succeeded()) {
    BlockCameraShake = BlockShakeAsset.Class;
    HitCameraShake = BlockShakeAsset.Class; // 被打時也用同一個 Shake
  }

  // --- Niagara 特效 ---
  static ConstructorHelpers::FObjectFinder<UNiagaraSystem>
      PerfectParryNiagaraAsset(
          TEXT("/Game/character_block_particle/"
               "NS_PerfectParrySpark.NS_PerfectParrySpark"));
  if (PerfectParryNiagaraAsset.Succeeded())
    PerfectParryNiagara = PerfectParryNiagaraAsset.Object;

  static ConstructorHelpers::FObjectFinder<UNiagaraSystem> BlockNiagaraAsset(
      TEXT("/Game/character_block_particle/NS_BlockSpark.NS_BlockSpark"));
  if (BlockNiagaraAsset.Succeeded())
    BlockNiagara = BlockNiagaraAsset.Object;

  // --- Input Actions ---
  static ConstructorHelpers::FObjectFinder<UInputAction> LockOnActionAsset(
      TEXT("/Game/ThirdPerson/Input/IA_LockOn.IA_LockOn"));
  if (LockOnActionAsset.Succeeded())
    LockOnAction = LockOnActionAsset.Object;

  static ConstructorHelpers::FObjectFinder<UInputAction> BlockActionAsset(
      TEXT("/Game/ThirdPerson/Input/Actions/IA_Block.IA_Block"));
  if (BlockActionAsset.Succeeded())
    BlockAction = BlockActionAsset.Object;

  static ConstructorHelpers::FObjectFinder<UInputAction> AttackActionAsset(
      TEXT("/Game/ThirdPerson/Input/Actions/IA_Attack.IA_Attack"));
  if (AttackActionAsset.Succeeded())
    AttackAction = AttackActionAsset.Object;

  static ConstructorHelpers::FObjectFinder<UInputAction> ExecutionActionAsset(
      TEXT("/Game/ThirdPerson/Input/Actions/IA_Execution.IA_Execution"));
  if (ExecutionActionAsset.Succeeded())
    ExecutionAction = ExecutionActionAsset.Object;

  // ===閃避系統===
  static ConstructorHelpers::FObjectFinder<UInputAction> DodgeActionAsset(
      TEXT("/Game/ThirdPerson/Input/Actions/IA_Dodge.IA_Dodge"));
  if (DodgeActionAsset.Succeeded())
    DodgeAction = DodgeActionAsset.Object;

  static ConstructorHelpers::FObjectFinder<UInputAction> JumpActionAsset(
      TEXT("/Game/ThirdPerson/Input/Actions/IA_Jump.IA_Jump"));
  if (JumpActionAsset.Succeeded())
    JumpAction = JumpActionAsset.Object;

  static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionAsset(
      TEXT("/Game/ThirdPerson/Input/Actions/IA_Move.IA_Move"));
  if (MoveActionAsset.Succeeded())
    MoveAction = MoveActionAsset.Object;

  static ConstructorHelpers::FObjectFinder<UInputAction> LookActionAsset(
      TEXT("/Game/ThirdPerson/Input/Actions/IA_Look.IA_Look"));
  if (LookActionAsset.Succeeded())
    LookAction = LookActionAsset.Object;

  static ConstructorHelpers::FObjectFinder<UInputMappingContext>
      DefaultMappingAsset(
          TEXT("/Game/ThirdPerson/Input/IMC_Default.IMC_Default"));
  if (DefaultMappingAsset.Succeeded())
    DefaultMappingContext = DefaultMappingAsset.Object;

  // ===== Minimap SceneCapture2D =====
  MinimapCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MinimapCapture"));
  MinimapCapture->SetupAttachment(RootComponent);
  MinimapCapture->SetRelativeLocation(FVector(0.f, 0.f, 3000.f)); // 3000 units above player
  MinimapCapture->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f)); // Face straight down
  MinimapCapture->ProjectionType = ECameraProjectionMode::Orthographic;
  MinimapCapture->OrthoWidth = 3000.f;
  MinimapCapture->bCaptureEveryFrame = false;     // Manual throttle in Tick
  MinimapCapture->bCaptureOnMovement = false;
  MinimapCapture->CaptureSource = SCS_FinalColorLDR;
  // Disable expensive render features
  MinimapCapture->ShowFlags.SetFog(false);
  MinimapCapture->ShowFlags.SetDynamicShadows(false);
  MinimapCapture->ShowFlags.SetBloom(false);
  MinimapCapture->ShowFlags.SetAmbientOcclusion(false);
  MinimapCapture->ShowFlags.SetDepthOfField(false);
  MinimapCapture->ShowFlags.SetMotionBlur(false);
  MinimapRenderTarget = nullptr; // Assigned via Blueprint property
}

void ASekiroCharacter::BeginPlay() {
  Super::BeginPlay();

  // 敵人（非玩家）：runtime 載入 WBP_OVERHEAD 並顯示頭頂 HUD
  // 玩家角色：隱藏 OverheadWidget（已有螢幕 HUD）
  if (OverheadWidget) {
    if (Cast<APlayerController>(GetController())) {
      OverheadWidget->SetVisibility(false);
      if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow,
          FString::Printf(TEXT("[%s] OverheadWidget HIDDEN (player)"), *GetName()));
    } else {
      // Runtime 載入 WBP_OVERHEAD（避免 CDO 覆蓋 ConstructorHelpers 的問題）
      UClass* WidgetCls = StaticLoadClass(
          UUserWidget::StaticClass(), nullptr,
          TEXT("/Game/WBP_OVERHEAD.WBP_OVERHEAD_C"));
      if (WidgetCls) {
        OverheadWidget->SetWidgetClass(WidgetCls);
        OverheadWidget->InitWidget();  // 強制重新初始化 Widget
        OverheadWidget->SetVisibility(true);
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
            FString::Printf(TEXT("[%s] OverheadWidget SET to WBP_OVERHEAD + InitWidget OK"), *GetName()));

        // --- MANUALLY BIND TO ACTOR ---
        if (USekiroWidgetBase* SekiroUI = Cast<USekiroWidgetBase>(OverheadWidget->GetUserWidgetObject())) {
            SekiroUI->BindToActor(this);
        } else if (GEngine) {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("[SekiroChar] Failed to cast UserWidgetObject to USekiroWidgetBase!"));
        }
      } else if (GEngine) {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
            TEXT("[SekiroChar] FAILED to load WBP_OVERHEAD!"));
      }
    }
  } else if (GEngine) {
    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
        FString::Printf(TEXT("[%s] OverheadWidget is NULL!"), *GetName()));
  }
  // --- 確保 C++ 成員指針有效（BP 設定的組件在 CDO 可能丟失） ---
  if (!CombatComponent) {
    CombatComponent = FindComponentByClass<USekiroCombatComponent>();
  }
  if (GEngine) {
    GEngine->AddOnScreenDebugMessage(-1, 8.0f, CombatComponent ? FColor::Green : FColor::Red,
      FString::Printf(TEXT("[%s] CombatComponent: %s"),
        *GetName(), CombatComponent ? TEXT("OK") : TEXT("NULL")));
  }

  // === Patchouli 式單骨架設定 ===
  // GetMesh() = 可見的 Reimu VRM 骨架（直接，無 VRMMesh 雙骨架）
  // 若存在 VRMMesh（隊友遺留），直接隱藏
  {
    static const FName VRMMeshCompName(TEXT("VRMMesh"));
    // 右手首：VRM 標準右手腕骨骼名稱（直接寫字符避免 \u 轉義問題）
    static const FName VRMRightHandBone = FName("右手首");

    FName ActualSocket = WeaponSocketName; // 預設：hand_r

    TArray<USkeletalMeshComponent*> SkelMeshes;
    GetComponents<USkeletalMeshComponent>(SkelMeshes);
    for (USkeletalMeshComponent* C : SkelMeshes) {
      if (C && C->GetFName() == VRMMeshCompName) {
        // 找到隊友遺留的 VRMMesh → 隱藏（Reimu 已在 GetMesh() 上）
        C->SetVisibility(false);
        C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        break;
      }
    }

    // 確認 GetMesh() 有 VRM 右手首骨骼 → 用它附加武器
    if (GetMesh() && GetMesh()->GetBoneIndex(VRMRightHandBone) != INDEX_NONE)
      ActualSocket = VRMRightHandBone;

    // 確保 GetMesh() 可見（Reimu 外觀）
    if (GetMesh())
      GetMesh()->SetVisibility(true, true);

    if (BlockWeaponPivot && GetMesh()) {
      BlockWeaponPivot->AttachToComponent(
          GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
          ActualSocket);
    }
    if (GEngine)
      GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
        FString::Printf(TEXT("[SekiroChar] Weapon socket=%s"), *ActualSocket.ToString()));
  }
  // （設定由 set_character_properties 或 batch_retarget_reimu.py 完成，不需要 SetHiddenInGame）




  if (WeaponMesh) {
    // Only override weapon mesh if still using the default Cube from constructor.
    // If the BP already has a custom mesh (e.g. KatanaMesh + M_Katana), respect it.
    UStaticMesh* CurrentMesh = WeaponMesh->GetStaticMesh();
    bool bIsDefaultCube = !CurrentMesh ||
        CurrentMesh->GetPathName().Contains(TEXT("BasicShapes/Cube"));
    if (bIsDefaultCube) {
      UStaticMesh *SwordMesh = Cast<UStaticMesh>(StaticLoadObject(
          UStaticMesh::StaticClass(), nullptr,
          TEXT("/Game/Sword_Animations/Demo/Mannequin/Character/Mesh/"
               "Sword.Sword")));
      if (SwordMesh) {
        WeaponMesh->SetStaticMesh(SwordMesh);
        WeaponMesh->SetWorldScale3D(FVector(1.f, 1.f, 1.f));
        WeaponMesh->SetRelativeRotation(FRotator(180.f, 0.f, 0.f));
      }
    } else {
      // Custom mesh (KatanaMesh etc.) — use rotation from BP editor, no override
    }
  }

  // --- 自動填充 ComboMontages ---
  if (CombatComponent) {
    // 偵測骨架類型，決定載入哪套 Montage
    USkeletalMesh* SKM = GetMesh() ? GetMesh()->GetSkeletalMeshAsset() : nullptr;
    FString SKPath = SKM ? SKM->GetPathName().ToLower() : TEXT("");
    const bool bIsReimuSkeleton = SKPath.Contains(TEXT("reimu")) || SKPath.Contains(TEXT("\u970a\u5922")); // reimu / 霊夢

    // 若現有 Montage 骨架不符（例如 BP 裡殘留舊 Mannequin 版），強制清除讓 auto-fill 重新載入
    if (CombatComponent->ComboMontages.Num() > 0) {
      UAnimMontage* First = CombatComponent->ComboMontages[0];
      if (First) {
        FString MontagePath = First->GetPathName().ToLower();
        bool bMontageMismatch = (bIsReimuSkeleton && !MontagePath.Contains(TEXT("reimu"))) ||
                                (!bIsReimuSkeleton && MontagePath.Contains(TEXT("reimu")));
        if (bMontageMismatch) {
          CombatComponent->ComboMontages.Empty();
          if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange,
              TEXT("[SekiroChar] Cleared incompatible ComboMontages (skeleton mismatch)"));
        }
      }
    }

    if (GEngine)
      GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan,
        FString::Printf(TEXT("[%s] ComboMontages count: %d (Reimu=%d)"),
          *GetName(), CombatComponent->ComboMontages.Num(), bIsReimuSkeleton));

    if (CombatComponent->ComboMontages.Num() == 0) {
      // Reimu（玩家）用 Combo_Attack_03_Reimu，Patchouli（敵人）用 Combo_Attack_01_Patchouli
      const TCHAR* ComboSuffix = bIsReimuSkeleton ? TEXT("_Reimu") : TEXT("_Patchouli");
      const TCHAR* ComboNum    = bIsReimuSkeleton ? TEXT("03") : TEXT("01");
      for (int32 i = 1; i <= 4; i++) {
        FString Path = FString(TEXT("/Game/Combo_Attack_")) + ComboNum + TEXT("_0") + FString::FromInt(i)
            + TEXT("_Seq_Montage") + ComboSuffix
            + TEXT(".Combo_Attack_") + ComboNum + TEXT("_0") + FString::FromInt(i)
            + TEXT("_Seq_Montage") + ComboSuffix;

        UAnimMontage* M = Cast<UAnimMontage>(
            StaticLoadObject(UAnimMontage::StaticClass(), nullptr, *Path));
        if (M) {
          CombatComponent->ComboMontages.Add(M);
        } else if (GEngine) {
          GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red,
            FString::Printf(TEXT("FAILED to load montage: %s"), *Path));
        }
      }
      if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green,
          FString::Printf(TEXT("Auto-loaded %d ComboMontages (%s)"),
            CombatComponent->ComboMontages.Num(),
            bIsReimuSkeleton ? TEXT("Reimu") : TEXT("Patchouli")));
    }

    // --- 自動填充 SpecialMontages (僅限敵人) ---
    if (!bIsReimuSkeleton && CombatComponent->SpecialMontages.Num() == 0) {
      UAnimMontage* SlashM = Cast<UAnimMontage>(
        StaticLoadObject(UAnimMontage::StaticClass(), nullptr, TEXT("/Game/boss_anim_retarget/AM_Perilous_Slash_Patchouli.AM_Perilous_Slash_Patchouli")));
      UAnimMontage* ThrustM = Cast<UAnimMontage>(
        StaticLoadObject(UAnimMontage::StaticClass(), nullptr, TEXT("/Game/boss_anim_retarget/AM_Perilous_Thrust_Patchouli.AM_Perilous_Thrust_Patchouli")));
      
      if (SlashM) CombatComponent->SpecialMontages.Add(SlashM);
      if (ThrustM) CombatComponent->SpecialMontages.Add(ThrustM);

      if (GEngine && CombatComponent->SpecialMontages.Num() > 0) {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Magenta,
          FString::Printf(TEXT("Auto-loaded %d SpecialMontages (Boss)"), CombatComponent->SpecialMontages.Num()));
      }
    }
  }


  // Add Input Mapping Context
  if (APlayerController *PlayerController =
          Cast<APlayerController>(Controller)) {
    if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                PlayerController->GetLocalPlayer())) {
      if (DefaultMappingContext) {
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
      }
    }
  }

  if (PostureComponent) {
    PostureComponent->OnPostureBroken.AddDynamic(
        this, &ASekiroCharacter::OnPostureBroken);
  }

  if (DeflectComponent) {
    DeflectComponent->OnParryResult.AddDynamic(
        this, &ASekiroCharacter::HandleParryResult);
  }

  if (AttributeComponent) {
    AttributeComponent->OnDeath.AddDynamic(this, &ASekiroCharacter::OnDeath);
  }

  // Cache CameraBoom SocketOffset（解鎖時還原相機位置）
  if (CameraBoom) {
    DefaultCameraBoomSocketOffset = CameraBoom->SocketOffset;
  }

  if (CombatComponent) {
    CombatComponent->OnExecutionTriggered.AddDynamic(
        this, &ASekiroCharacter::OnExecutionTriggered);
    CombatComponent->OnAttackStarted.AddDynamic(
        this, &ASekiroCharacter::OnAttackStartedForTrail);
    CombatComponent->OnAttackEnded.AddDynamic(
        this, &ASekiroCharacter::OnAttackEndedForTrail);
  }

  // === 武器 Re-Attach 到 Reimu VRM 右手骨骼 ===
  // 只對 Player（有 APlayerController）生效，敵人 SekiroEnemy 不受影響
  // 注意：面向修正在 Blueprint 的 VRMMesh Rotation 裡設定，不在 C++ 做
  if (Cast<APlayerController>(GetController())) {
    TArray<USkeletalMeshComponent*> SkelComps;
    GetComponents<USkeletalMeshComponent>(SkelComps);
    for (USkeletalMeshComponent* Comp : SkelComps) {
      if (Comp && Comp != GetMesh() && Comp->GetSkeletalMeshAsset()) {
        // --- 武器掛載到 VRM 右手骨骼 ---
        if (BlockWeaponPivot) {
          FName BoneName = FName(TEXT("\u53f3\u624b\u9996"));  // 右手首
          if (Comp->DoesSocketExist(BoneName)) {
            BlockWeaponPivot->AttachToComponent(
                Comp,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                BoneName);
            if (GEngine) {
              GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green,
                  FString::Printf(TEXT("[Reimu] Weapon on %s"),
                                  *BoneName.ToString()));
            }
          } else {
            if (GEngine) {
              GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
                  FString::Printf(TEXT("[Reimu] Bone '%s' NOT FOUND!"),
                                  *BoneName.ToString()));
            }
          }
        }
        break;
      }
    }
  }
  // === END 武器 Re-Attach ===

  // Load checkpoint save data on start (restores which checkpoints were activated)
  LoadCheckpointSaveData();

  // ===== Minimap: Assign RenderTarget after BeginPlay =====
  if (MinimapCapture && MinimapRenderTarget)
  {
    MinimapCapture->TextureTarget = MinimapRenderTarget;
    MinimapCapture->CaptureScene(); // Initial capture
  }
}


void ASekiroCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // ===== Minimap Throttled Capture (~10fps) =====
  if (MinimapCapture && MinimapRenderTarget && MinimapCapture->TextureTarget)
  {
    MinimapCaptureTimer += DeltaTime;
    if (MinimapCaptureTimer >= MinimapCaptureInterval)
    {
      MinimapCaptureTimer = 0.f;
      MinimapCapture->CaptureScene();
    }
  }

  if (GEngine) {
    float CurrentHP =
        AttributeComponent ? AttributeComponent->CurrentHealth : -1.0f;
    float MaxHP = AttributeComponent ? AttributeComponent->MaxHealth : -1.0f;

    float CurrentPostureValue =
        PostureComponent ? PostureComponent->CurrentPosture : -1.0f;
    float MaxPostureValue =
        PostureComponent ? PostureComponent->MaxPosture : -1.0f;

    const bool bPostureBroken = PostureComponent && MaxPostureValue > 0.0f &&
                                CurrentPostureValue >= MaxPostureValue;

    const FString DebugLine =
        FString::Printf(TEXT("PLAYER  HP: %.0f/%.0f  |  Posture: %.0f/%.0f  |  "
                             "Blocking: %s  |  Broken: %s"),
                        CurrentHP, MaxHP, CurrentPostureValue, MaxPostureValue,
                        bIsBlocking ? TEXT("YES") : TEXT("NO"),
                        bPostureBroken ? TEXT("YES") : TEXT("NO"));

    GEngine->AddOnScreenDebugMessage(
        10, 0.f, bPostureBroken ? FColor::Red : FColor::Green, DebugLine);
  }

  // 擋刀時轉「Pivot」而唔係直接轉 WeaponMesh，刀先會打橫且唔會被動畫蓋過
  USceneComponent *RotateTarget = BlockWeaponComponent
                                      ? BlockWeaponComponent.Get()
                                      : BlockWeaponPivot.Get();



  if (RotateTarget) {
    UAnimInstance *Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    const bool bInBlockMontage =
        (Anim &&
         ((ParryAttemptMontage &&
           Anim->Montage_IsPlaying(ParryAttemptMontage)) ||
          (BlockLoopMontage && Anim->Montage_IsPlaying(BlockLoopMontage)) ||
          (BlockHitMontage && Anim->Montage_IsPlaying(BlockHitMontage)) ||
          (BlockEndMontage && Anim->Montage_IsPlaying(BlockEndMontage))));
    const bool bApplyBlockRot = bIsBlocking || bInBlockMontage;
    RotateTarget->SetRelativeRotation(bApplyBlockRot
                                          ? BlockWeaponRotationWhenBlocking
                                          : FRotator::ZeroRotator);
  }

  // 鎖定時：鏡頭／面向跟住目標；目標死或太遠則自動解除
  if (bIsLockedOn && LockedTarget) {
    if (!IsValid(LockedTarget)) {
      LockedTarget = nullptr;
      bIsLockedOn = false;
    } else {
      const float DistSq = FVector::DistSquared(
          GetActorLocation(), LockedTarget->GetActorLocation());
      if (DistSq > LockOnRange * LockOnRange) {
        LockedTarget = nullptr;
        bIsLockedOn = false;
      } else {
        APlayerController *PC = Cast<APlayerController>(Controller);
        if (PC) {
          bUseControllerRotationYaw = true;
          if (GetCharacterMovement())
            GetCharacterMovement()->bOrientRotationToMovement = false;

          FVector ViewLoc;
          FRotator ViewRot;
          PC->GetPlayerViewPoint(ViewLoc, ViewRot);

          const FVector TargetLoc = LockedTarget->GetActorLocation() +
                                    FVector(0.f, 0.f, LockOnTargetZOffset);
          if (CameraBoom) {
            FVector NewOffset = DefaultCameraBoomSocketOffset;
            NewOffset.Z =
                DefaultCameraBoomSocketOffset.Z + LockOnCameraSocketOffsetZ;
            CameraBoom->SocketOffset = NewOffset;
          }
          const FVector ToTarget = TargetLoc - ViewLoc;
          if (!ToTarget.IsNearlyZero()) {
            FRotator Desired = ToTarget.Rotation();
            Desired.Roll = 0.f;
            if (bLockOnUseFixedPitch) {
              Desired.Pitch = LockOnFixedPitch;
            } else {
              Desired.Pitch =
                  FMath::Clamp(Desired.Pitch, LockOnPitchMin, LockOnPitchMax);
            }

            FRotator Current = PC->GetControlRotation();
            Current.Roll = 0.f;
            if (bLockOnUseFixedPitch) {
              Current.Pitch = LockOnFixedPitch;
            } else {
              Current.Pitch =
                  FMath::Clamp(Current.Pitch, LockOnPitchMin, LockOnPitchMax);
            }

            FRotator NewRot = FMath::RInterpTo(Current, Desired, DeltaTime,
                                               LockOnRotationSpeed);
            if (bLockOnUseFixedPitch) {
              NewRot.Pitch = LockOnFixedPitch;
            }
            PC->SetControlRotation(NewRot);
          }
        }
      }
    }
  } else {
    bUseControllerRotationYaw = false;
    if (GetCharacterMovement())
      GetCharacterMovement()->bOrientRotationToMovement = true;
    if (CameraBoom) {
      CameraBoom->SocketOffset = DefaultCameraBoomSocketOffset;
    }
  }

  // Outline（描邊）：鎖定目標開 Custom Depth，解除時關
  if (PreviousLockedTarget && PreviousLockedTarget != LockedTarget) {
    TArray<UPrimitiveComponent *> Comps;
    PreviousLockedTarget->GetComponents(Comps);
    for (UPrimitiveComponent *C : Comps) {
      if (C) {
        C->SetRenderCustomDepth(false);
      }
    }
    PreviousLockedTarget = nullptr;
  }
  if (LockedTarget) {
    TArray<UPrimitiveComponent *> Comps;
    LockedTarget->GetComponents(Comps);
    for (UPrimitiveComponent *C : Comps) {
      if (C) {
        C->SetRenderCustomDepth(true);
        C->SetCustomDepthStencilValue(1);
      }
    }
    PreviousLockedTarget = LockedTarget;
  }

  // 敵人（非玩家）自動面向玩家
  APlayerController *PC = Cast<APlayerController>(GetController());
  if (!PC && bFacePlayerAsAI) {
    APawn *PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn && PlayerPawn != this) {
      const float DistSq = FVector::DistSquared(GetActorLocation(),
                                                PlayerPawn->GetActorLocation());
      if (DistSq <= 3000.f * 3000.f) {
        FVector ToPlayer = (PlayerPawn->GetActorLocation() - GetActorLocation())
                               .GetSafeNormal2D();
        if (!ToPlayer.IsNearlyZero()) {
          FRotator Desired = ToPlayer.Rotation();
          SetActorRotation(
              FMath::RInterpTo(GetActorRotation(), Desired, DeltaTime, 12.f));
        }
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASekiroCharacter::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  if (UEnhancedInputComponent *EnhancedInputComponent =
          Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

    // Jumping
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this,
                                       &ACharacter::Jump);
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed,
                                       this, &ACharacter::StopJumping);

    // Moving
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered,
                                       this, &ASekiroCharacter::Move);

    // Looking
    EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,
                                       this, &ASekiroCharacter::Look);

    // Blocking
    if (BlockAction) {
      UE_LOG(LogTemp, Warning, TEXT("Binding BlockAction: %s"),
             *BlockAction->GetName());
      EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Started,
                                         this, &ASekiroCharacter::StartBlock);
      // 重要：按住期間每幀都保持 Blocking（避免 Completed 抖動令 bIsBlocking
      // 掉落）
      EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Triggered,
                                         this, &ASekiroCharacter::StartBlock);
      EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Completed,
                                         this, &ASekiroCharacter::StopBlock);
    } else {
      UE_LOG(LogTemp, Error,
             TEXT("BlockAction is NULL in SetupPlayerInputComponent!"));
    }

    // Attacking
    EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started,
                                       this, &ASekiroCharacter::Attack);

    // ===閃避系統===
    if (DodgeAction)
    {
      EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started,
                                         this, &ASekiroCharacter::Dodge);
    }
    else
    {
      UE_LOG(LogTemp, Warning, TEXT("DodgeAction is NULL in SetupPlayerInputComponent!"));
    }

    // --- Vibe Coding: Dynamic Checkpoint Intercept ---
    FInputKeyBinding FKeyBinding(FInputChord(EKeys::F), IE_Pressed);
    FKeyBinding.bExecuteWhenPaused = true;
    FKeyBinding.KeyDelegate.GetDelegateForManualSet().BindLambda([this]() {
        if (APlayerController* PC = Cast<APlayerController>(this->GetController())) {
            if (PC->IsPaused()) {
                // If paused, assume we are in the menu. Close it!
                for (TObjectIterator<UUserWidget> It; It; ++It) {
                    if (It->GetWorld() == this->GetWorld() && It->GetName().Contains(TEXT("WBP_UpgradeMenu"))) {
                        It->RemoveFromParent();
                    }
                }
                PC->SetPause(false);
                PC->bShowMouseCursor = false;
                FInputModeGameOnly GameMode;
                PC->SetInputMode(GameMode);
                PC->SetViewTargetWithBlend(this, 0.5f);
                if (GEngine) GEngine->AddOnScreenDebugMessage(1337, 0.0f, FColor::Cyan, TEXT("")); // Clear HUD
                return;
            }
            
            TArray<AActor*> OverlappingActors;
            this->GetOverlappingActors(OverlappingActors);
            for (AActor* OverlappingActor : OverlappingActors) {
                if (OverlappingActor->GetName().Contains(TEXT("Checkpoint"))) {
                    // Save checkpoint activation to disk (Phase 5 Respawn System)
                    this->SaveCheckpointActivated(OverlappingActor->GetName());

                    if (UClass* WidgetClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/Game/Blueprints/UI/WBP_UpgradeMenu.WBP_UpgradeMenu_C"))) {
                        if (UUserWidget* UpgradeMenu = CreateWidget<UUserWidget>(this->GetWorld(), WidgetClass)) {
                            UpgradeMenu->AddToViewport(9999);
                            PC->bShowMouseCursor = true;
                            
                            FInputModeGameAndUI InputMode;
                            InputMode.SetWidgetToFocus(UpgradeMenu->TakeWidget());
                            InputMode.SetHideCursorDuringCapture(false);
                            PC->SetInputMode(InputMode);
                            
                            PC->SetViewTargetWithBlend(OverlappingActor, 0.5f);
                            PC->SetPause(true);

                            // Draw Stats HUD
                            if (GEngine) {
                                FString StatsStr = TEXT("\n=== CURRENT STATS ===");
                                if (this->AttributeComponent) StatsStr += FString::Printf(TEXT("\n[1] Health: %.0f / %.0f"), this->AttributeComponent->CurrentHealth, this->AttributeComponent->MaxHealth);
                                if (this->CombatComponent) StatsStr += FString::Printf(TEXT("\n[2] Attack Power: %.0f"), this->CombatComponent->AttackPostureDamage);
                                if (this->PostureComponent) StatsStr += FString::Printf(TEXT("\n[3] Max Posture: %.0f"), this->PostureComponent->MaxPosture);
                                StatsStr += TEXT("\n=====================\nPress 1, 2, or 3 to Upgrade.\nPress F to Close.");
                                GEngine->AddOnScreenDebugMessage(1337, 9999.f, FColor::Cyan, StatsStr, true, FVector2D(1.5f, 1.5f));
                            }
                        }
                    }
                    break;
                }
            }
        }
    });
    PlayerInputComponent->KeyBindings.Add(FKeyBinding);

    // --- Vibe Coding: Keyboard Upgrade Shortcuts for UI Bypass ---
    FInputKeyBinding Key1Binding(FInputChord(EKeys::One), IE_Pressed);
    Key1Binding.bExecuteWhenPaused = true;
    Key1Binding.KeyDelegate.GetDelegateForManualSet().BindLambda([this]() {
        if (APlayerController* PC = Cast<APlayerController>(this->GetController())) {
            if (PC->IsPaused() && this->AttributeComponent) {
                this->AttributeComponent->MaxHealth += 50.0f;
                this->AttributeComponent->CurrentHealth = this->AttributeComponent->MaxHealth;
                this->AttributeComponent->OnHealthChanged.Broadcast(this->AttributeComponent->CurrentHealth, this->AttributeComponent->MaxHealth);
                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Upgraded Health! (Keyboard 1)"));

                // Refresh HUD
                if (GEngine) {
                    FString StatsStr = TEXT("\n=== CURRENT STATS ===");
                    StatsStr += FString::Printf(TEXT("\n[1] Health: %.0f / %.0f"), this->AttributeComponent->CurrentHealth, this->AttributeComponent->MaxHealth);
                    if (this->CombatComponent) StatsStr += FString::Printf(TEXT("\n[2] Attack Power: %.0f"), this->CombatComponent->AttackPostureDamage);
                    if (this->PostureComponent) StatsStr += FString::Printf(TEXT("\n[3] Max Posture: %.0f"), this->PostureComponent->MaxPosture);
                    StatsStr += TEXT("\n=====================\nPress 1, 2, or 3 to Upgrade.\nPress F to Close.");
                    GEngine->AddOnScreenDebugMessage(1337, 9999.f, FColor::Cyan, StatsStr, true, FVector2D(1.5f, 1.5f));
                }
            }
        }
    });
    PlayerInputComponent->KeyBindings.Add(Key1Binding);

    FInputKeyBinding Key2Binding(FInputChord(EKeys::Two), IE_Pressed);
    Key2Binding.bExecuteWhenPaused = true;
    Key2Binding.KeyDelegate.GetDelegateForManualSet().BindLambda([this]() {
        if (APlayerController* PC = Cast<APlayerController>(this->GetController())) {
            if (PC->IsPaused() && this->CombatComponent) {
                this->CombatComponent->AttackPostureDamage += 10.0f;
                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Upgraded Attack! (Keyboard 2)"));

                // Refresh HUD
                if (GEngine) {
                    FString StatsStr = TEXT("\n=== CURRENT STATS ===");
                    if (this->AttributeComponent) StatsStr += FString::Printf(TEXT("\n[1] Health: %.0f / %.0f"), this->AttributeComponent->CurrentHealth, this->AttributeComponent->MaxHealth);
                    StatsStr += FString::Printf(TEXT("\n[2] Attack Power: %.0f"), this->CombatComponent->AttackPostureDamage);
                    if (this->PostureComponent) StatsStr += FString::Printf(TEXT("\n[3] Max Posture: %.0f"), this->PostureComponent->MaxPosture);
                    StatsStr += TEXT("\n=====================\nPress 1, 2, or 3 to Upgrade.\nPress F to Close.");
                    GEngine->AddOnScreenDebugMessage(1337, 9999.f, FColor::Cyan, StatsStr, true, FVector2D(1.5f, 1.5f));
                }
            }
        }
    });
    PlayerInputComponent->KeyBindings.Add(Key2Binding);

    FInputKeyBinding Key3Binding(FInputChord(EKeys::Three), IE_Pressed);
    Key3Binding.bExecuteWhenPaused = true;
    Key3Binding.KeyDelegate.GetDelegateForManualSet().BindLambda([this]() {
        if (APlayerController* PC = Cast<APlayerController>(this->GetController())) {
            if (PC->IsPaused() && this->PostureComponent) {
                this->PostureComponent->MaxPosture += 50.0f;
                this->PostureComponent->OnPostureChanged.Broadcast(this->PostureComponent->CurrentPosture, this->PostureComponent->MaxPosture);
                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Upgraded Posture! (Keyboard 3)"));

                // Refresh HUD
                if (GEngine) {
                    FString StatsStr = TEXT("\n=== CURRENT STATS ===");
                    if (this->AttributeComponent) StatsStr += FString::Printf(TEXT("\n[1] Health: %.0f / %.0f"), this->AttributeComponent->CurrentHealth, this->AttributeComponent->MaxHealth);
                    if (this->CombatComponent) StatsStr += FString::Printf(TEXT("\n[2] Attack Power: %.0f"), this->CombatComponent->AttackPostureDamage);
                    StatsStr += FString::Printf(TEXT("\n[3] Max Posture: %.0f"), this->PostureComponent->MaxPosture);
                    StatsStr += TEXT("\n=====================\nPress 1, 2, or 3 to Upgrade.\nPress F to Close.");
                    GEngine->AddOnScreenDebugMessage(1337, 9999.f, FColor::Cyan, StatsStr, true, FVector2D(1.5f, 1.5f));
                }
            }
        }
    });
    PlayerInputComponent->KeyBindings.Add(Key3Binding);

    // --- Debug: Press 0 five times to instantly kill the player ---
    FInputKeyBinding Key0Binding(FInputChord(EKeys::Zero), IE_Pressed);
    Key0Binding.bExecuteWhenPaused = false;
    Key0Binding.KeyDelegate.GetDelegateForManualSet().BindLambda([this]() {
        static int32 DebugKillCount = 0;
        DebugKillCount++;
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow,
            FString::Printf(TEXT("[DEBUG] Kill in %d..."), 5 - DebugKillCount));
        if (DebugKillCount >= 5)
        {
            DebugKillCount = 0;
            if (AttributeComponent && !bIsDead)
            {
                AttributeComponent->ApplyDamage(99999.f);
            }
        }
    });
    PlayerInputComponent->KeyBindings.Add(Key0Binding);

    // --- Space key: respawn when dead ---
    FInputKeyBinding SpaceBinding(FInputChord(EKeys::SpaceBar), IE_Pressed);
    SpaceBinding.bExecuteWhenPaused = true;
    SpaceBinding.KeyDelegate.GetDelegateForManualSet().BindLambda([this]() {
        if (bIsDead)
        {
            DoRespawn();
        }
    });
    PlayerInputComponent->KeyBindings.Add(SpaceBinding);
    // --------------------------------------------------

    // Execution
    EnhancedInputComponent->BindAction(ExecutionAction, ETriggerEvent::Started,
                                       this, &ASekiroCharacter::Execution);

    // Lock-on
    if (LockOnAction) {
      EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started,
                                         this,
                                         &ASekiroCharacter::LockOnPressed);
    }

    // --- Full-Screen Map Toggle (M key, works when paused) ---
    FInputKeyBinding MapKeyBinding(FInputChord(EKeys::M), IE_Pressed);
    MapKeyBinding.bExecuteWhenPaused = true;
    MapKeyBinding.KeyDelegate.GetDelegateForManualSet().BindLambda([this]() {
        ToggleMap();
    });
    PlayerInputComponent->KeyBindings.Add(MapKeyBinding);
  } else {
    UE_LOG(
        LogTemp, Error,
        TEXT("'%s' Failed to find an Enhanced Input component! This template "
             "is built to use the Enhanced Input system. If you intend to use "
             "the legacy system, then you will need to update this C++ file."),
        *GetNameSafe(this));
  }
}

void ASekiroCharacter::ToggleMap()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    if (bMapOpen)
    {
        // ── Close map ──────────────────────────────────────────
        if (MapOverlayWidget)
        {
            MapOverlayWidget->RemoveFromParent();
            MapOverlayWidget = nullptr;
        }

        // Restore HUD
        if (CachedGameHUDWidget)
        {
            CachedGameHUDWidget->SetVisibility(ESlateVisibility::Visible);
        }

        // Reset MinimapCapture back to the character (it follows via attachment)
        if (MinimapCapture)
        {
            MinimapCapture->SetRelativeLocation(FVector(0.f, 0.f, 600.f));
        }

        UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
        PC->SetInputMode(FInputModeGameOnly());
        PC->SetShowMouseCursor(false);
        bMapOpen = false;
    }
    else
    {
        // ── Open map ───────────────────────────────────────────

        // Find and hide the game HUD
        if (GameHUDWidgetClass && !CachedGameHUDWidget)
        {
            // Try to find an existing instance of the HUD widget
            TArray<UUserWidget*> Found;
            UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), Found, GameHUDWidgetClass, false);
            if (Found.Num() > 0)
                CachedGameHUDWidget = Found[0];
        }
        if (CachedGameHUDWidget)
        {
            CachedGameHUDWidget->SetVisibility(ESlateVisibility::Collapsed);
        }

        if (MapOverlayWidgetClass)
        {
            MapOverlayWidget = CreateWidget<UUserWidget>(PC, MapOverlayWidgetClass);
            if (MapOverlayWidget)
            {
                MapOverlayWidget->AddToViewport(10); // above HUD
                UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.0f);
                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(MapOverlayWidget->TakeWidget());
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                PC->SetInputMode(InputMode);
                PC->SetShowMouseCursor(true);
                bMapOpen = true;
            }
        }
    }
}

void ASekiroCharacter::PanMapCapture(float WorldDeltaX, float WorldDeltaY)
{
    if (!MinimapCapture || !bMapOpen) return;

    // WorldDeltaX/Y are already in world units (cm)
    // +WorldX = map pans "up" (north), +WorldY = map pans "right" (east)
    FVector Loc = MinimapCapture->GetComponentLocation();
    Loc.X += WorldDeltaX;
    Loc.Y += WorldDeltaY;
    MinimapCapture->SetWorldLocation(Loc);

    // Force an immediate render into the RenderTarget while the game is paused
    MinimapCapture->CaptureScene();
}

void ASekiroCharacter::Move(const FInputActionValue &Value) {
  // 完全唔鎖移動（畀被擊中、擋刀、任何時候都可以行），除非角色死咗
  if (AttributeComponent && AttributeComponent->CurrentHealth <= 0.f)
    return;

  // 確保 CharacterMovement 在地面時係 Walking mode，唔會被 Montage 停止
  // ⚠️ 修復：只有在地面上才強制 Walking，空中跳躍時不覆蓋 MOVE_Falling（否則角色漂浮）
  if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround()) {
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
  }

  // input is a Vector2D
  FVector2D MovementVector = Value.Get<FVector2D>();

  if (Controller != nullptr) {
    // find out which way is forward
    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    // get forward vector
    const FVector ForwardDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    // get right vector
    const FVector RightDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    // add movement
    AddMovementInput(ForwardDirection, MovementVector.Y);
    AddMovementInput(RightDirection, MovementVector.X);
  }
}

void ASekiroCharacter::Look(const FInputActionValue &Value) {
  // input is a Vector2D
  FVector2D LookAxisVector = Value.Get<FVector2D>();

  if (Controller != nullptr) {
    // add yaw and pitch input to controller
    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(LookAxisVector.Y);
  }
}

void ASekiroCharacter::StartBlock() {
  if (!DeflectComponent)
    return;

  // HUD 閃爍 - 右鍵
  if (USekiroGameHUDWidget* HUD = USekiroGameHUDWidget::GetInstance())
    HUD->FlashWidgetByName(FName("Img_Block"));
  // 只喺第一次按下時更新時間同播動畫（Triggered 每幀呼叫唔應該重複做）
  if (!bIsBlocking) {
    LastBlockInputTimeSeconds =
        GetWorld() ? GetWorld()->GetTimeSeconds() : LastBlockInputTimeSeconds;

    DeflectComponent->StartBlocking();
    bIsBlocking = true;
    Tags.AddUnique(FName("State.Combat.HoldingBlock"));

    // BlockStart：播完後由 OnBlockStartMontageEnded 接 BlockLoop
    if (ParryAttemptMontage) {
      UAnimInstance *Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
      if (Anim) {
        FOnMontageEnded EndDel;
        EndDel.BindUObject(this, &ASekiroCharacter::OnBlockStartMontageEnded);
        Anim->Montage_SetEndDelegate(EndDel, ParryAttemptMontage);
        PlayAnimMontage(ParryAttemptMontage);
      } else {
        PlayAnimMontage(ParryAttemptMontage);
        if (BlockLoopMontage)
          PlayAnimMontage(BlockLoopMontage);
      }
    } else if (BlockLoopMontage) {
      PlayAnimMontage(BlockLoopMontage);
    }
  }
  // 按住期間：只保持 bIsBlocking = true（唔更新時間、唔重播動畫）
}

void ASekiroCharacter::StopBlock() {
  if (!DeflectComponent)
    return;

  // 如果根本冇喺 Blocking 狀態，直接返回
  if (!bIsBlocking)
    return;

  // 保護：避免 Enhanced Input trigger 導致一按即 Completed 令擋格立即取消
  if (GetWorld()) {
    const double Now = GetWorld()->GetTimeSeconds();
    if ((Now - LastBlockInputTimeSeconds) < 0.05) {
      return;
    }
  }

  DeflectComponent->StopBlocking();
  bIsBlocking = false;
  Tags.Remove(FName("State.Combat.HoldingBlock"));

  UAnimInstance *Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
  if (Anim) {
    if (ParryAttemptMontage)
      Anim->Montage_Stop(0.2f, ParryAttemptMontage);
    if (BlockLoopMontage)
      Anim->Montage_Stop(0.2f, BlockLoopMontage);
    if (BlockHitMontage)
      Anim->Montage_Stop(0.2f, BlockHitMontage);
  }
  if (BlockEndMontage) {
    PlayAnimMontage(BlockEndMontage);
  }
}

void ASekiroCharacter::Attack() {
  if (!CombatComponent) {
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
      FString::Printf(TEXT("[%s] Attack(): CombatComponent NULL!"), *GetName()));
    return;
  }

  if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Magenta,
    FString::Printf(TEXT("[%s] Attack() called, ComboMontages=%d"),
      *GetName(), CombatComponent->ComboMontages.Num()));

  // HUD 閃爍 - 左鍵
  if (USekiroGameHUDWidget* HUD = USekiroGameHUDWidget::GetInstance())
    HUD->FlashWidgetByName(FName("Img_Attack"));

  // 若可處決（架勢條滿）則直接處決，否則攻擊
  if (CombatComponent->RequestExecution()) {
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple,
      TEXT("Attack(): RequestExecution returned true, skipping attack"));
    return;
  }
  CombatComponent->RequestAttack();
}

void ASekiroCharacter::LockOnPressed() { ToggleLockOn(); }

void ASekiroCharacter::ToggleLockOn() {
  // HUD 閃爍 - 鎖定
  if (USekiroGameHUDWidget* HUD = USekiroGameHUDWidget::GetInstance())
    HUD->FlashWidgetByName(FName("Img_LockOn"));

  UWorld *World = GetWorld();
  if (!World)
    return;

  if (bIsLockedOn) {
    LockedTarget = nullptr;
    bIsLockedOn = false;
    return;
  }

  TArray<AActor *> Candidates;
  UGameplayStatics::GetAllActorsWithTag(World, LockOnTargetTag, Candidates);

  const FVector MyLoc = GetActorLocation();
  const FVector Forward = GetActorForwardVector();
  AActor *Best = nullptr;
  float BestScore = -1.f;

  for (AActor *Actor : Candidates) {
    if (!Actor || Actor == this)
      continue;
    const float DistSq = FVector::DistSquared(MyLoc, Actor->GetActorLocation());
    if (DistSq > LockOnRange * LockOnRange)
      continue;
    FVector ToActor = (Actor->GetActorLocation() - MyLoc).GetSafeNormal2D();
    const float Dot = FVector::DotProduct(Forward, ToActor);
    if (Dot < 0.3f)
      continue; // 要喺前方一定角度內
    const float Score =
        Dot / (1.f + FMath::Sqrt(DistSq) * 0.01f); // 愈近、愈正前方愈好
    if (Score > BestScore) {
      BestScore = Score;
      Best = Actor;
    }
  }

  if (Best) {
    LockedTarget = Best;
    bIsLockedOn = true;
  }
}

void ASekiroCharacter::Execution(const FInputActionValue &Value) {
  if (CombatComponent)
    CombatComponent->RequestExecution();
  // Montage 由 OnExecutionTriggered 播放
}

void ASekiroCharacter::OnPostureBroken() {
  if (DeathblowWidget) {
    DeathblowWidget->SetVisibility(true);
  }

  // Add Stunned Tag
  Tags.Add(FName("State.Stunned"));

  // Optional: Disable movement or AI logic here
  if (GetCharacterMovement()) {
    GetCharacterMovement()->StopMovementImmediately();
  }

  if (StunMontage) {
    PlayAnimMontage(StunMontage);
  }
}

void ASekiroCharacter::HandleParryResult(EParryResult Result) {
  // 取得自己（防守方）武器位置
  const FVector DefenderWeaponLoc =
      WeaponMesh ? WeaponMesh->GetComponentLocation() : GetActorLocation();

  // 嘗試搵攻擊方（敵人）嘅武器位置，用嚟喺敵人嗰邊都生成火花
  FVector AttackerWeaponLoc = DefenderWeaponLoc; // 預設用防守方位置
  if (bIsLockedOn && LockedTarget) {
    // 搵敵人嘅 WeaponMesh
    ASekiroCharacter *EnemyChar = Cast<ASekiroCharacter>(LockedTarget);
    if (EnemyChar && EnemyChar->WeaponMesh) {
      AttackerWeaponLoc = EnemyChar->WeaponMesh->GetComponentLocation();
    } else {
      AttackerWeaponLoc = LockedTarget->GetActorLocation() +
                          LockedTarget->GetActorForwardVector() * 80.f;
    }
  }

  // 中間點：兩把刀碰撞嘅位置（用嚟生成火花最自然）
  const FVector ClashPoint = (DefenderWeaponLoc + AttackerWeaponLoc) * 0.5f;

  // === Helper: 執行 Hit Stop（暫停效果）===
  auto DoHitStop = [this](float Duration, float TimeScale) {
    if (Duration > 0.f && TimeScale > 0.f) {
      UWorld *W = GetWorld();
      if (W && W->GetWorldSettings()) {
        W->GetWorldSettings()->TimeDilation = TimeScale;
        FTimerHandle H;
        float GameDuration = Duration * TimeScale;
        W->GetTimerManager().SetTimer(
            H,
            [W]() {
              if (W && W->GetWorldSettings())
                W->GetWorldSettings()->TimeDilation = 1.f;
            },
            GameDuration, false);
      }
    }
  };

  // === Helper: Camera Shake ===
  auto DoCameraShake = [this](TSubclassOf<UCameraShakeBase> ShakeClass,
                              const FString &DebugLabel) {
    if (ShakeClass) {
      APlayerController *PC = Cast<APlayerController>(GetController());
      if (PC && PC->PlayerCameraManager) {
        PC->PlayerCameraManager->StartCameraShake(ShakeClass, 1.0f);
        if (GEngine)
          GEngine->AddOnScreenDebugMessage(
              -1, 2.0f, FColor::Magenta,
              FString::Printf(TEXT("Camera Shake: %s ✓"), *DebugLabel));
      } else {
        if (GEngine)
          GEngine->AddOnScreenDebugMessage(
              -1, 2.0f, FColor::Red,
              FString::Printf(
                  TEXT("Camera Shake FAIL: %s (No PC/CameraManager)"),
                  *DebugLabel));
      }
    } else {
      if (GEngine)
        GEngine->AddOnScreenDebugMessage(
            -1, 2.0f, FColor::Red,
            FString::Printf(TEXT("Camera Shake FAIL: %s (ShakeClass=None)"),
                            *DebugLabel));
    }
  };

  // === Helper: 喺指定位置生成攻擊方火花 ===
  auto DoAttackerSpark = [this](const FVector &Loc) {
    if (AttackerSparkNiagara) {
      UNiagaraFunctionLibrary::SpawnSystemAtLocation(
          GetWorld(), AttackerSparkNiagara, Loc, FRotator::ZeroRotator,
          FVector(2.f), true, true);
    }
  };

  // === 舊版 Cascade 粒子（兼容）===
  auto DoLegacyParticle = [this](const FVector &Loc) {
    if (ParryBlockParticle) {
      UGameplayStatics::SpawnEmitterAtLocation(
          GetWorld(), ParryBlockParticle, Loc, FRotator::ZeroRotator, true);
    }
  };

  switch (Result) {
  case EParryResult::Perfect: {
    // 動畫
    if (ParrySuccessMontage)
      PlayAnimMontage(ParrySuccessMontage);

    // 音效 — 精準格擋嘅清脆金屬聲
    if (PerfectParrySound)
      UGameplayStatics::PlaySoundAtLocation(this, PerfectParrySound,
                                            ClashPoint);

    // 火花 — 精準格擋用明亮大火花
    if (PerfectParryNiagara)
      UNiagaraFunctionLibrary::SpawnSystemAtLocation(
          GetWorld(), PerfectParryNiagara, ClashPoint, FRotator::ZeroRotator,
          FVector(3.f), true, true);

    // 敵人嗰邊都生成火花
    DoAttackerSpark(AttackerWeaponLoc);

    // 舊版粒子
    DoLegacyParticle(ClashPoint);

    // Camera Shake — 精準格擋較大震動
    DoCameraShake(PerfectParryCameraShake, TEXT("PerfectParry"));

    // Hit Stop — 精準格擋較強嘅時間暫停
    DoHitStop(PerfectParryHitStopDuration, PerfectParryHitStopTimeScale);

    // Blueprint Feedback
    OnParryBlockFeedback();

    if (GEngine)
      GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow,
                                       TEXT("★ PERFECT PARRY! ★"));
    break;
  }
  case EParryResult::Blocked: {
    // 音效 — 普通格擋嘅金屬碰撞聲
    if (BlockSound)
      UGameplayStatics::PlaySoundAtLocation(this, BlockSound, ClashPoint);

    // 火花 — 普通格擋用細啲暗啲嘅火花
    if (BlockNiagara)
      UNiagaraFunctionLibrary::SpawnSystemAtLocation(
          GetWorld(), BlockNiagara, ClashPoint, FRotator::ZeroRotator,
          FVector(2.f), true, true);

    // 敵人嗰邊都生成火花
    DoAttackerSpark(AttackerWeaponLoc);

    // 舊版粒子
    DoLegacyParticle(ClashPoint);

    // Camera Shake — 普通格擋較輕震動
    DoCameraShake(BlockCameraShake, TEXT("Block"));

    // Hit Stop — 普通格擋較弱嘅時間暫停
    DoHitStop(BlockHitStopDuration, BlockHitStopTimeScale);

    // Blueprint Feedback
    OnParryBlockFeedback();

    // 播放格擋被擊中動畫
    if (BlockHitMontage) {
      UAnimInstance *Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
      if (Anim) {
        FOnMontageEnded EndDel;
        EndDel.BindUObject(this, &ASekiroCharacter::OnBlockHitMontageEnded);
        Anim->Montage_SetEndDelegate(EndDel, BlockHitMontage);
      }
      PlayAnimMontage(BlockHitMontage);
    }
    break;
  }
  case EParryResult::Failed:
    if (HitMontage)
      PlayAnimMontage(HitMontage);
    // Camera Shake — 被直接打中（沒有擋住）
    DoCameraShake(HitCameraShake, TEXT("Hit(Failed)"));
    break;
  }
}

void ASekiroCharacter::OnBlockStartMontageEnded(UAnimMontage *Montage,
                                                bool bInterrupted) {
  if (bInterrupted || !bIsBlocking)
    return;
  if (BlockLoopMontage)
    PlayAnimMontage(BlockLoopMontage);
}

void ASekiroCharacter::OnBlockHitMontageEnded(UAnimMontage *Montage,
                                              bool bInterrupted) {
  if (!bIsBlocking)
    return;
  if (BlockLoopMontage)
    PlayAnimMontage(BlockLoopMontage);
}

void ASekiroCharacter::OnExecutionTriggered(AActor *Target) {
  // 進入無敵狀態
  if (AttributeComponent) {
    AttributeComponent->bIsInvincible = true;
  }

  // 停止格擋狀態（防止處決後卡在格擋動畫）
  if (bIsBlocking) {
    StopBlock();
  }

  // 停止所有正在播放的動畫
  UAnimInstance *Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
  if (Anim) {
    Anim->Montage_StopGroupByName(0.2f, FName("DefaultGroup"));
  }

  // 重置 Combo 狀態
  if (CombatComponent) {
    CombatComponent->ResetCombo();
  }

  // 播放處決動畫
  if (ExecutionMontage) {
    float Duration = PlayAnimMontage(ExecutionMontage);
    
    // 處決動畫結束後取消無敵
    if (Duration > 0.0f) {
      FTimerHandle InvincibilityTimer;
      GetWorldTimerManager().SetTimer(
          InvincibilityTimer,
          [this]() {
            if (AttributeComponent) {
              AttributeComponent->bIsInvincible = false;
            }
          },
          Duration, false);
    } else {
      // 動畫播放失敗，立即取消無敵
      if (AttributeComponent)
        AttributeComponent->bIsInvincible = false;
    }
  }

  // 處決音效
  if (ExecutionSound) {
    FVector SoundLoc = Target ? Target->GetActorLocation() : GetActorLocation();
    UGameplayStatics::PlaySoundAtLocation(this, ExecutionSound, SoundLoc);
  }
}

void ASekiroCharacter::OnAttackStartedForTrail() {
  K2_OnAttackStarted_Implementation();
}
void ASekiroCharacter::OnAttackEndedForTrail() {
  K2_OnAttackEnded_Implementation();
}

void ASekiroCharacter::OnParryBlockFeedback_Implementation() {}

void ASekiroCharacter::K2_OnAttackStarted_Implementation() {}

void ASekiroCharacter::K2_OnAttackEnded_Implementation() {}

void ASekiroCharacter::OnDeath() {
  // Prevent double-death
  if (bIsDead) return;
  bIsDead = true;

  // Cache death location for nearest-checkpoint calculation
  CachedDeathLocation = GetActorLocation();

  // === 1. 停止所有移動 ===
  if (GetCharacterMovement()) {
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
  }

  // === 2. 停止 AI 攻擊行為（敵人專用）===
  USekiroEnemyAttributeComponent* EnemyAI =
      FindComponentByClass<USekiroEnemyAttributeComponent>();
  if (EnemyAI) {
    EnemyAI->bAutoAttack = false;
    EnemyAI->SetComponentTickEnabled(false);
  }

  // === 3. 禁用玩家輸入（玩家專用）===
  APlayerController* PC = Controller ? Cast<APlayerController>(Controller) : nullptr;
  if (PC) {
    DisableInput(PC);
  }

  // === 4. 停止所有正在播放的動畫 ===
  UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
  if (Anim) {
    Anim->Montage_Stop(0.1f);
  }

  // === 5. 停止格擋狀態 ===
  if (bIsBlocking) {
    if (DeflectComponent) {
      DeflectComponent->StopBlocking();
    }
    bIsBlocking = false;
  }

  // === 6. 重置戰鬥狀態 ===
  if (CombatComponent) {
    CombatComponent->ResetCombo();
  }

  // === 7. 播放死亡動畫 ===
  if (DeathMontage) {
    float Duration = PlayAnimMontage(DeathMontage);

    if (Duration > 0.0f) {
      // 在 BlendOut 開始之前凍結動畫（提前 0.3 秒）
      float FreezeTime = FMath::Max(Duration - 0.3f, 0.1f);
      FTimerHandle DeathFreezeTimer;
      GetWorldTimerManager().SetTimer(
          DeathFreezeTimer,
          [this]() {
            // 凍結動畫在最後一幀（保持倒地姿勢）
            if (GetMesh()) {
              GetMesh()->bPauseAnims = true;
              GetMesh()->bNoSkeletonUpdate = true;
            }

            // 禁用碰撞（不再阻擋玩家移動）
            if (GetCapsuleComponent()) {
              GetCapsuleComponent()->SetCollisionEnabled(
                  ECollisionEnabled::NoCollision);
            }

            // 停止 Tick（節省效能）
            SetActorTickEnabled(false);
          },
          FreezeTime, false);
    }
  }

  // === 8. Fade to black + show You Died UI (player only) ===
  if (PC)
  {
    // Start camera fade to black over 2 seconds immediately on death
    if (PC->PlayerCameraManager)
    {
        PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, 2.0f, FLinearColor::Black, false, true);
    }

    // After 2s: show widget + debug text, re-enable input for Space key
    FTimerHandle YouDiedShowTimer;
    GetWorldTimerManager().SetTimer(YouDiedShowTimer, [this, PC]()
    {
        // Create the YouDied widget (if class assigned)
        if (YouDiedWidgetClass)
        {
            YouDiedWidgetInstance = CreateWidget<UUserWidget>(PC, YouDiedWidgetClass);
            if (YouDiedWidgetInstance)
            {
                YouDiedWidgetInstance->AddToViewport(10);

                // Try to bind a Button named "RespawnButton"
                UButton* RespawnBtn = Cast<UButton>(
                    YouDiedWidgetInstance->GetWidgetFromName(TEXT("RespawnButton")));
                if (RespawnBtn)
                {
                    RespawnBtn->OnClicked.AddDynamic(this, &ASekiroCharacter::DoRespawn);
                }
            }
        }

        // Re-enable input so Space key binding fires (keep movement disabled)
        if (PC->GetPawn())
        {
            EnableInput(PC);
        }
        FInputModeGameOnly GameMode;
        PC->SetInputMode(GameMode);
        PC->SetShowMouseCursor(false);
    }, 2.0f, false);
  }
}


// version 3 — 2026年2月22日 23:50 (香港時間)
// v1：加處決音效、被打Camera Shake、DoCameraShake Debug訊息
// v2：降低鎖定敵人時Camera高度
// v3：用 ConstructorHelpers 自動載入所有音效/CameraShake/Niagara/InputAction
// v4 (Phase 5)：加 Global Respawn System、SaveGame、You Died UI

// ======================== RESPAWN SYSTEM (Phase 5) ========================

void ASekiroCharacter::DoRespawn()
{
    APlayerController* PC = Controller ? Cast<APlayerController>(Controller) : nullptr;

    // Step 0: Fade from black
    if (PC && PC->PlayerCameraManager)
    {
        PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, 0.8f, FLinearColor::Black, false, false);
    }

    // Clear any on-screen death messages
    if (GEngine) GEngine->RemoveOnScreenDebugMessage(9001);

    // Step 1: Remove the You Died widget
    if (YouDiedWidgetInstance)
    {
        YouDiedWidgetInstance->RemoveFromParent();
        YouDiedWidgetInstance = nullptr;
    }

    // Step 2: Find nearest activated checkpoint using actor tag "Checkpoint"
    TArray<AActor*> CheckpointActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Checkpoint"), CheckpointActors);

    AActor* NearestCheckpoint = nullptr;
    float MinDist = MAX_FLT;

    for (AActor* CP : CheckpointActors)
    {
        if (CP && IsCheckpointActivated(CP->GetName()))
        {
            float Dist = FVector::Dist(CachedDeathLocation, CP->GetActorLocation());
            if (Dist < MinDist)
            {
                MinDist = Dist;
                NearestCheckpoint = CP;
            }
        }
    }

    // Step 3: Determine respawn location (checkpoint or PlayerStart fallback)
    FVector RespawnLocation = FVector::ZeroVector;
    bool bFoundLocation = false;

    if (NearestCheckpoint)
    {
        RespawnLocation = NearestCheckpoint->GetActorLocation() + FVector(200.f, 0.f, 100.f);
        bFoundLocation = true;
        UE_LOG(LogTemp, Log, TEXT("[FYP] Respawning at checkpoint: %s"), *NearestCheckpoint->GetName());
    }
    else
    {
        AActor* PlayerStartActor = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
        if (PlayerStartActor)
        {
            RespawnLocation = PlayerStartActor->GetActorLocation() + FVector(0.f, 0.f, 100.f);
            bFoundLocation = true;
            UE_LOG(LogTemp, Log, TEXT("[FYP] No activated checkpoint — respawning at PlayerStart."));
        }
    }

    // Step 4: Unfreeze animation
    if (GetMesh())
    {
        GetMesh()->bPauseAnims = false;
        GetMesh()->bNoSkeletonUpdate = false;
    }

    // Step 5: Re-enable capsule collision
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // Step 6: Re-enable movement
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
    }

    // Step 7: Restore health to full (MaxHealth preserves Phase 4 upgrade bonuses)
    if (AttributeComponent)
    {
        AttributeComponent->CurrentHealth = AttributeComponent->MaxHealth;
        // Broadcast so HUD updates immediately
        AttributeComponent->OnHealthChanged.Broadcast(
            AttributeComponent->CurrentHealth, AttributeComponent->MaxHealth);
    }

    // Step 8: Reset posture
    if (PostureComponent)
    {
        PostureComponent->ResetPosture();
    }

    // Step 9: Reset combat state
    if (CombatComponent)
    {
        CombatComponent->ResetCombo();
    }

    // Step 10: Teleport
    if (bFoundLocation)
    {
        SetActorLocation(RespawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
    }

    // Step 11: Re-enable tick
    SetActorTickEnabled(true);

    // Step 12: Re-enable input and reset to game input mode
    if (PC)
    {
        EnableInput(PC);
        PC->SetInputMode(FInputModeGameOnly());
        PC->SetShowMouseCursor(false);
    }

    // Step 13: Reset dead flag
    bIsDead = false;

    UE_LOG(LogTemp, Log, TEXT("[FYP] Respawn complete."));
}

void ASekiroCharacter::LoadCheckpointSaveData()
{
    UFYPSaveGame* SaveGame = Cast<UFYPSaveGame>(
        UGameplayStatics::LoadGameFromSlot(CheckpointSaveSlot, 0));
    if (SaveGame)
    {
        ActivatedCheckpointNames = SaveGame->ActivatedCheckpointNames;
        UE_LOG(LogTemp, Log, TEXT("[FYP] Loaded %d activated checkpoints from save."),
               ActivatedCheckpointNames.Num());
    }
    else
    {
        ActivatedCheckpointNames.Empty();
        UE_LOG(LogTemp, Log, TEXT("[FYP] No save data found — starting fresh."));
    }
}

void ASekiroCharacter::SaveCheckpointActivated(const FString& CheckpointName)
{
    if (!ActivatedCheckpointNames.Contains(CheckpointName))
    {
        ActivatedCheckpointNames.Add(CheckpointName);
    }

    UFYPSaveGame* SaveGame = Cast<UFYPSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UFYPSaveGame::StaticClass()));
    if (SaveGame)
    {
        SaveGame->ActivatedCheckpointNames = ActivatedCheckpointNames;
        UGameplayStatics::SaveGameToSlot(SaveGame, CheckpointSaveSlot, 0);
        UE_LOG(LogTemp, Log, TEXT("[FYP] Saved checkpoint activation: %s"), *CheckpointName);
    }
}

bool ASekiroCharacter::IsCheckpointActivated(const FString& CheckpointName) const
{
    return ActivatedCheckpointNames.Contains(CheckpointName);
}

// ===閃避系統===
void ASekiroCharacter::Dodge()
{
    // 1. Cooldown 及 Dodge 中 → 忽略輸入
    if (!bCanDodge || bIsDodging)
    {
        return;
    }

    // 2. 確保 AttributeComponent 存在（直接用成員指針，無需 GetComponentByClass）
    if (!AttributeComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DODGE] AttributeComponent is NULL — cannot set invincibility!"));
        return;
    }

    // 3. 設定狀態
    bIsDodging = true;
    bCanDodge  = false;
    AttributeComponent->bIsInvincible = true;

    // 4. 計算 Dash 方向
    //    GetLastMovementInputVector() 返回上一幀 AddMovementInput 的世界方向
    //    Lock-on 時角色面向 Boss，GetActorForwardVector() 指向 Boss
    //    無 WASD 輸入時 → 向後退（遠離 Boss）
    FVector DodgeDir = GetLastMovementInputVector();
    if (DodgeDir.IsNearlyZero())
    {
        DodgeDir = -GetActorForwardVector();  // 預設：遠離 Boss 方向
    }
    DodgeDir.Z = 0.0f;
    DodgeDir.Normalize();

    // 5. 施加瞬間位移（XY 覆蓋，不覆蓋 Z 保持重力）
    LaunchCharacter(DodgeDir * DodgeLaunchSpeed, true, false);

    // ===閃避動畫===
    if (DodgeMontage)
    {
        PlayAnimMontage(DodgeMontage, 1.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("[DODGE] Dir=%s | Invincible=%.1fs | Cooldown=%.1fs"),
           *DodgeDir.ToString(), DodgeDuration, DodgeCooldown);

    // 6. DodgeDuration 後解除無敵幀
    GetWorld()->GetTimerManager().SetTimer(
        DodgeInvincibilityHandle,
        [this]()
        {
            bIsDodging = false;
            if (AttributeComponent)
            {
                AttributeComponent->bIsInvincible = false;
            }
        },
        DodgeDuration, false);

    // 7. DodgeCooldown 後允許再次 Dodge
    GetWorld()->GetTimerManager().SetTimer(
        DodgeCooldownHandle,
        [this]()
        {
            bCanDodge = true;
        },
        DodgeCooldown, false);
}
