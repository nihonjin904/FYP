#include "Components/SekiroEnemyAttributeComponent.h"
#include "Components/SekiroCombatComponent.h"
#include "Characters/SekiroCharacter.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

USekiroEnemyAttributeComponent::USekiroEnemyAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USekiroEnemyAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	CombatComp = GetOwner()->FindComponentByClass<USekiroCombatComponent>();
}

void USekiroEnemyAttributeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ComboTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ComboTimerHandle);
		}
		ComboTimerHandle.Invalidate();
	}
	Super::EndPlay(EndPlayReason);
}

void USekiroEnemyAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 敵人面向玩家（打主角而唔係打一個方向）
	if (bFacePlayer)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		AActor* OwnerActor = GetOwner();
		if (PlayerPawn && OwnerActor && PlayerPawn != OwnerActor)
		{
			float DistSq = FVector::DistSquared(OwnerActor->GetActorLocation(), PlayerPawn->GetActorLocation());
			if (DistSq <= FacePlayerRange * FacePlayerRange)
			{
				FVector ToPlayer = (PlayerPawn->GetActorLocation() - OwnerActor->GetActorLocation()).GetSafeNormal2D();
				if (ToPlayer.IsNormalized())
				{
					FRotator DesiredYaw = ToPlayer.Rotation();
					FRotator Current = OwnerActor->GetActorRotation();
					float Speed = FacePlayerSpeed * DeltaTime;
					FRotator NewRot = FMath::RInterpTo(Current, DesiredYaw, DeltaTime, FacePlayerSpeed);
					OwnerActor->SetActorRotation(NewRot);
				}
			}
		}
	}

	if (bAutoAttack && CombatComp && !ComboTimerHandle.IsValid())
	{
		TimeSinceLastAttack += DeltaTime;
		if (TimeSinceLastAttack >= AttackInterval)
		{
			TimeSinceLastAttack = 0.0f;
			StartComboAttackCycle();
		}
	}
}

void USekiroEnemyAttributeComponent::StartComboAttackCycle()
{
	ASekiroCharacter* SekiroChar = Cast<ASekiroCharacter>(GetOwner());
	if (!SekiroChar)
	{
		return;
	}

	// First attack immediately
	SekiroChar->Attack();

	const int32 NumFollowUpAttacks = FMath::Max(0, ComboAttackCount - 1);
	if (NumFollowUpAttacks <= 0)
	{
		return;
	}

	ComboAttacksRemaining = NumFollowUpAttacks;
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		ComboTimerHandle,
		this,
		&USekiroEnemyAttributeComponent::OnComboAttackTimer,
		ComboAttackInterval,
		true
	);
}

void USekiroEnemyAttributeComponent::OnComboAttackTimer()
{
	ASekiroCharacter* SekiroChar = Cast<ASekiroCharacter>(GetOwner());
	if (SekiroChar)
	{
		SekiroChar->Attack();
	}

	--ComboAttacksRemaining;
	if (ComboAttacksRemaining <= 0)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(ComboTimerHandle);
		}
		ComboTimerHandle.Invalidate();
	}
}
