
#include "Characters/SekiroCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/SekiroAttributeComponent.h"
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
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/PrimitiveComponent.h"


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

  // BlockWeaponPivot：手 (hand_r) → Pivot → WeaponMesh，擋刀時只轉 Pivot 就唔會被動畫蓋過
  BlockWeaponPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BlockWeaponPivot"));
  BlockWeaponPivot->SetupAttachment(GetMesh(), FName("hand_r"));

  WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
  WeaponMesh->SetupAttachment(BlockWeaponPivot); // 掛喺 Pivot 下面，唔直接掛 hand_r
  WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(
      TEXT("/Engine/BasicShapes/Cube.Cube"));
  if (CubeMeshAsset.Succeeded()) {
    WeaponMesh->SetStaticMesh(CubeMeshAsset.Object);
    WeaponMesh->SetWorldScale3D(FVector(0.1f, 0.1f, 1.0f));
  }
}

void ASekiroCharacter::BeginPlay() {
  Super::BeginPlay();

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
    CombatComponent->OnExecutionTriggered.AddDynamic(this, &ASekiroCharacter::OnExecutionTriggered);
    CombatComponent->OnAttackStarted.AddDynamic(this, &ASekiroCharacter::OnAttackStartedForTrail);
    CombatComponent->OnAttackEnded.AddDynamic(this, &ASekiroCharacter::OnAttackEndedForTrail);
  }
}

void ASekiroCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (GEngine) {
    float CurrentHP = AttributeComponent ? AttributeComponent->CurrentHealth : -1.0f;
    float MaxHP = AttributeComponent ? AttributeComponent->MaxHealth : -1.0f;

    float CurrentPostureValue =
        PostureComponent ? PostureComponent->CurrentPosture : -1.0f;
    float MaxPostureValue =
        PostureComponent ? PostureComponent->MaxPosture : -1.0f;

    const bool bPostureBroken =
        PostureComponent && MaxPostureValue > 0.0f &&
        CurrentPostureValue >= MaxPostureValue;

    const FString DebugLine = FString::Printf(
        TEXT("PLAYER  HP: %.0f/%.0f  |  Posture: %.0f/%.0f  |  Blocking: %s  |  Broken: %s"),
        CurrentHP, MaxHP, CurrentPostureValue, MaxPostureValue,
        bIsBlocking ? TEXT("YES") : TEXT("NO"),
        bPostureBroken ? TEXT("YES") : TEXT("NO"));

    GEngine->AddOnScreenDebugMessage(10, 0.f,
                                     bPostureBroken ? FColor::Red
                                                    : FColor::Green,
                                     DebugLine);
  }

  // 擋刀時轉「Pivot」而唔係直接轉 WeaponMesh，刀先會打橫且唔會被動畫蓋過
  USceneComponent* RotateTarget = BlockWeaponComponent ? BlockWeaponComponent.Get() : BlockWeaponPivot.Get();
  if (RotateTarget) {
    // 就算 bIsBlocking 因 Enhanced Input Trigger 抖動，都用 Montage 狀態保持打橫
    UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    const bool bInBlockMontage =
      (Anim && (
        (ParryAttemptMontage && Anim->Montage_IsPlaying(ParryAttemptMontage)) ||
        (BlockLoopMontage && Anim->Montage_IsPlaying(BlockLoopMontage)) ||
        (BlockHitMontage && Anim->Montage_IsPlaying(BlockHitMontage)) ||
        (BlockEndMontage && Anim->Montage_IsPlaying(BlockEndMontage))
      ));
    const bool bApplyBlockRot = bIsBlocking || bInBlockMontage;
    RotateTarget->SetRelativeRotation(bApplyBlockRot ? BlockWeaponRotationWhenBlocking : FRotator::ZeroRotator);
  }

  // 鎖定時：鏡頭／面向跟住目標；目標死或太遠則自動解除
  if (bIsLockedOn && LockedTarget) {
    if (!IsValid(LockedTarget)) {
      LockedTarget = nullptr;
      bIsLockedOn = false;
    } else {
      const float DistSq = FVector::DistSquared(GetActorLocation(), LockedTarget->GetActorLocation());
      if (DistSq > LockOnRange * LockOnRange) {
        LockedTarget = nullptr;
        bIsLockedOn = false;
      } else {
        APlayerController* PC = Cast<APlayerController>(Controller);
        if (PC) {
          // 鎖定時角色一定面向目標：用 Controller Yaw 控制角色方向（唔跟移動方向）
          bUseControllerRotationYaw = true;
          if (GetCharacterMovement())
            GetCharacterMovement()->bOrientRotationToMovement = false;

          // 用視角位置指向目標（避免向下鎖地），用 RInterpTo 避免 360 轉圈
          FVector ViewLoc;
          FRotator ViewRot;
          PC->GetPlayerViewPoint(ViewLoc, ViewRot);

          // 抬高瞄準點，避免鎖地面；同時抬高 Camera Boom 位置
          const FVector TargetLoc = LockedTarget->GetActorLocation() + FVector(0.f, 0.f, LockOnTargetZOffset);
          if (CameraBoom) {
            FVector NewOffset = DefaultCameraBoomSocketOffset;
            NewOffset.Z = DefaultCameraBoomSocketOffset.Z + LockOnCameraSocketOffsetZ;
            CameraBoom->SocketOffset = NewOffset;
          }
          const FVector ToTarget = TargetLoc - ViewLoc;
          if (!ToTarget.IsNearlyZero()) {
            FRotator Desired = ToTarget.Rotation();
            Desired.Roll = 0.f;
            if (bLockOnUseFixedPitch) {
              Desired.Pitch = LockOnFixedPitch;
            } else {
              Desired.Pitch = FMath::Clamp(Desired.Pitch, LockOnPitchMin, LockOnPitchMax);
            }

            FRotator Current = PC->GetControlRotation();
            Current.Roll = 0.f;
            if (bLockOnUseFixedPitch) {
              Current.Pitch = LockOnFixedPitch;
            } else {
              Current.Pitch = FMath::Clamp(Current.Pitch, LockOnPitchMin, LockOnPitchMax);
            }

            FRotator NewRot = FMath::RInterpTo(Current, Desired, DeltaTime, LockOnRotationSpeed);
            if (bLockOnUseFixedPitch) {
              NewRot.Pitch = LockOnFixedPitch;
            }
            PC->SetControlRotation(NewRot);
          }
        }
      }
    }
  }
  else {
    // 無鎖定時還原：角色跟移動方向轉向
    bUseControllerRotationYaw = false;
    if (GetCharacterMovement())
      GetCharacterMovement()->bOrientRotationToMovement = true;
    if (CameraBoom) {
      CameraBoom->SocketOffset = DefaultCameraBoomSocketOffset;
    }
  }

  // Outline（描邊）：鎖定目標開 Custom Depth，解除時關
  if (PreviousLockedTarget && PreviousLockedTarget != LockedTarget) {
    TArray<UPrimitiveComponent*> Comps;
    PreviousLockedTarget->GetComponents(Comps);
    for (UPrimitiveComponent* C : Comps) {
      if (C) { C->SetRenderCustomDepth(false); }
    }
    PreviousLockedTarget = nullptr;
  }
  if (LockedTarget) {
    TArray<UPrimitiveComponent*> Comps;
    LockedTarget->GetComponents(Comps);
    for (UPrimitiveComponent* C : Comps) {
      if (C) { C->SetRenderCustomDepth(true); C->SetCustomDepthStencilValue(1); }
    }
    PreviousLockedTarget = LockedTarget;
  }

  // 敵人（非玩家）自動面向玩家
  APlayerController* PC = Cast<APlayerController>(GetController());
  if (!PC && bFacePlayerAsAI) {
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn && PlayerPawn != this) {
      const float DistSq = FVector::DistSquared(GetActorLocation(), PlayerPawn->GetActorLocation());
      if (DistSq <= 3000.f * 3000.f) {
        FVector ToPlayer = (PlayerPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
        if (!ToPlayer.IsNearlyZero()) {
          FRotator Desired = ToPlayer.Rotation();
          SetActorRotation(FMath::RInterpTo(GetActorRotation(), Desired, DeltaTime, 12.f));
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
      // 重要：按住期間每幀都保持 Blocking（避免 Completed 抖動令 bIsBlocking 掉落）
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
                                         this, &ASekiroCharacter::LockOnPressed);
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
  if (!DeflectComponent) return;

  LastBlockInputTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : LastBlockInputTimeSeconds;
  DeflectComponent->StartBlocking();
  bIsBlocking = true;
  Tags.AddUnique(FName("State.Combat.HoldingBlock"));

  // BlockStart：播完後由 OnBlockStartMontageEnded 接 BlockLoop
  if (ParryAttemptMontage) {
    UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (Anim) {
      FOnMontageEnded EndDel;
      EndDel.BindUObject(this, &ASekiroCharacter::OnBlockStartMontageEnded);
      Anim->Montage_SetEndDelegate(EndDel, ParryAttemptMontage);
      PlayAnimMontage(ParryAttemptMontage);
    } else {
      PlayAnimMontage(ParryAttemptMontage);
      if (BlockLoopMontage) PlayAnimMontage(BlockLoopMontage);
    }
  } else if (BlockLoopMontage) {
    PlayAnimMontage(BlockLoopMontage);
  }
}

void ASekiroCharacter::StopBlock() {
  if (!DeflectComponent) return;

  // 保護：避免 Enhanced Input trigger 導致一按即 Completed 令擋格立即取消
  if (GetWorld()) {
    const double Now = GetWorld()->GetTimeSeconds();
    if ((Now - LastBlockInputTimeSeconds) < 0.10) {
      return;
    }
  }

  DeflectComponent->StopBlocking();
  bIsBlocking = false;
  Tags.Remove(FName("State.Combat.HoldingBlock"));

  // 只停止「擋格相關」Montage，唔好一口氣 Stop 所有（避免攻擊/受擊 Montage 異常）
  UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
  if (Anim) {
    if (ParryAttemptMontage) Anim->Montage_Stop(0.2f, ParryAttemptMontage);
    if (BlockLoopMontage) Anim->Montage_Stop(0.2f, BlockLoopMontage);
    if (BlockHitMontage) Anim->Montage_Stop(0.2f, BlockHitMontage);
  }
  if (BlockEndMontage) {
    PlayAnimMontage(BlockEndMontage);
  }
}

void ASekiroCharacter::Attack() {
  if (!CombatComponent) return;
  // 左鍵：若可處決（架勢條滿）則直接處決，否則攻擊
  if (CombatComponent->RequestExecution())
    return;
  CombatComponent->RequestAttack();
}

void ASekiroCharacter::LockOnPressed() {
  ToggleLockOn();
}

void ASekiroCharacter::ToggleLockOn() {
  UWorld* World = GetWorld();
  if (!World) return;

  if (bIsLockedOn) {
    LockedTarget = nullptr;
    bIsLockedOn = false;
    return;
  }

  TArray<AActor*> Candidates;
  UGameplayStatics::GetAllActorsWithTag(World, LockOnTargetTag, Candidates);

  const FVector MyLoc = GetActorLocation();
  const FVector Forward = GetActorForwardVector();
  AActor* Best = nullptr;
  float BestScore = -1.f;

  for (AActor* Actor : Candidates) {
    if (!Actor || Actor == this) continue;
    const float DistSq = FVector::DistSquared(MyLoc, Actor->GetActorLocation());
    if (DistSq > LockOnRange * LockOnRange) continue;
    FVector ToActor = (Actor->GetActorLocation() - MyLoc).GetSafeNormal2D();
    const float Dot = FVector::DotProduct(Forward, ToActor);
    if (Dot < 0.3f) continue; // 要喺前方一定角度內
    const float Score = Dot / (1.f + FMath::Sqrt(DistSq) * 0.01f); // 愈近、愈正前方愈好
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
  auto DoParryFeedback = [this]() {
    if (ParryBlockSound) UGameplayStatics::PlaySoundAtLocation(this, ParryBlockSound, GetActorLocation());
    if (ParryBlockParticle) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParryBlockParticle, WeaponMesh ? WeaponMesh->GetComponentLocation() : GetActorLocation(), FRotator::ZeroRotator, true);
    if (ParryBlockNiagara) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ParryBlockNiagara, WeaponMesh ? WeaponMesh->GetComponentLocation() : GetActorLocation(), FRotator::ZeroRotator, FVector(1.f), true, true);
    OnParryBlockFeedback_Implementation();
    if (ParryHitStopDuration > 0.f && ParryHitStopTimeScale > 0.f) {
      UWorld* W = GetWorld();
      if (W && W->GetWorldSettings()) {
        W->GetWorldSettings()->TimeDilation = ParryHitStopTimeScale;
        FTimerHandle H;
        float GameDuration = ParryHitStopDuration * ParryHitStopTimeScale;
        W->GetTimerManager().SetTimer(H, [W]() { if (W && W->GetWorldSettings()) W->GetWorldSettings()->TimeDilation = 1.f; }, GameDuration, false);
      }
    }
  };
  switch (Result) {
  case EParryResult::Perfect:
    if (ParrySuccessMontage) PlayAnimMontage(ParrySuccessMontage);
    DoParryFeedback();
    break;
  case EParryResult::Blocked:
    DoParryFeedback();
    if (BlockHitMontage) {
      UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
      if (Anim) {
        FOnMontageEnded EndDel;
        EndDel.BindUObject(this, &ASekiroCharacter::OnBlockHitMontageEnded);
        Anim->Montage_SetEndDelegate(EndDel, BlockHitMontage);
      }
      PlayAnimMontage(BlockHitMontage);
    }
    break;
  case EParryResult::Failed:
    if (HitMontage) PlayAnimMontage(HitMontage);
    break;
  }
}

void ASekiroCharacter::OnBlockStartMontageEnded(UAnimMontage* Montage, bool bInterrupted) {
  if (bInterrupted || !bIsBlocking) return;
  if (BlockLoopMontage) PlayAnimMontage(BlockLoopMontage);
}

void ASekiroCharacter::OnBlockHitMontageEnded(UAnimMontage* Montage, bool bInterrupted) {
  if (!bIsBlocking) return;
  if (BlockLoopMontage) PlayAnimMontage(BlockLoopMontage);
}

void ASekiroCharacter::OnExecutionTriggered(AActor* Target) {
  if (ExecutionMontage)
    PlayAnimMontage(ExecutionMontage);
}

void ASekiroCharacter::OnAttackStartedForTrail() { K2_OnAttackStarted_Implementation(); }
void ASekiroCharacter::OnAttackEndedForTrail() { K2_OnAttackEnded_Implementation(); }

void ASekiroCharacter::OnParryBlockFeedback_Implementation() {}

void ASekiroCharacter::K2_OnAttackStarted_Implementation() {}

void ASekiroCharacter::K2_OnAttackEnded_Implementation() {}

void ASekiroCharacter::OnDeath() {
  if (GetCharacterMovement()) {
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
  }

  if (Controller) {
    DisableInput(Cast<APlayerController>(Controller));
  }

  if (DeathMontage) {
    PlayAnimMontage(DeathMontage);
  }
}
