
#include "Characters/SekiroCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
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
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/WorldSettings.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "UI/SekiroGameHUDWidget.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"


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
  OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);
  OverheadWidget->SetDrawAtDesiredSize(true);
  OverheadWidget->SetRelativeLocation(
      FVector(0.0f, 0.0f, 100.0f)); // Above head

  // Create Deathblow Widget
  DeathblowWidget =
      CreateDefaultSubobject<UWidgetComponent>(TEXT("DeathblowWidget"));
  DeathblowWidget->SetupAttachment(RootComponent);
  DeathblowWidget->SetWidgetSpace(EWidgetSpace::Screen);
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
}

void ASekiroCharacter::BeginPlay() {
  Super::BeginPlay();

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
    UStaticMesh *SwordMesh = Cast<UStaticMesh>(StaticLoadObject(
        UStaticMesh::StaticClass(), nullptr,
        TEXT("/Game/Sword_Animations/Demo/Mannequin/Character/Mesh/"
             "Sword.Sword")));
    if (SwordMesh) {
      WeaponMesh->SetStaticMesh(SwordMesh);
      WeaponMesh->SetWorldScale3D(FVector(1.f, 1.f, 1.f));
      WeaponMesh->SetRelativeRotation(FRotator(180.f, 0.f, 0.f));
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
      // Reimu（玩家）用 _Reimu 版，Patchouli（敵人）用 _Patchouli 版
      const TCHAR* ComboSuffix = bIsReimuSkeleton ? TEXT("_Reimu") : TEXT("_Patchouli");
      for (int32 i = 1; i <= 4; i++) {
        // FString::Printf 必須接受字面量格式字符串；用字符串拼接代替
        FString Path = FString::Printf(TEXT("/Game/Combo_Attack_01_0")) + FString::FromInt(i)
            + TEXT("_Seq_Montage") + ComboSuffix
            + TEXT(".Combo_Attack_01_0") + FString::FromInt(i)
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
}

void ASekiroCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

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

    // Execution
    EnhancedInputComponent->BindAction(ExecutionAction, ETriggerEvent::Started,
                                       this, &ASekiroCharacter::Execution);

    // Lock-on
    if (LockOnAction) {
      EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started,
                                         this,
                                         &ASekiroCharacter::LockOnPressed);
    }
  } else {
    UE_LOG(
        LogTemp, Error,
        TEXT("'%s' Failed to find an Enhanced Input component! This template "
             "is built to use the Enhanced Input system. If you intend to use "
             "the legacy system, then you will need to update this C++ file."),
        *GetNameSafe(this));
  }
}

void ASekiroCharacter::Move(const FInputActionValue &Value) {
  // 完全唔鎖移動（畀被擊中、擋刀、任何時候都可以行），除非角色死咗
  if (AttributeComponent && AttributeComponent->CurrentHealth <= 0.f)
    return;

  // 確保 CharacterMovement 永遠係 Walking mode，唔會被 Montage 停止
  if (GetCharacterMovement()) {
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
  if (Controller) {
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (PC) {
      DisableInput(PC);
    }
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
}

// version 3 — 2026年2月22日 23:50 (香港時間)
// v1：加處決音效、被打Camera Shake、DoCameraShake Debug訊息
// v2：降低鎖定敵人時Camera高度
// v3：用 ConstructorHelpers
// 自動載入所有音效/CameraShake/Niagara/InputAction（永遠不會因Blueprint重置而丟失）
