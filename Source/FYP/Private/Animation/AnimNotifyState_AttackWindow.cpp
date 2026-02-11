#include "Animation/AnimNotifyState_AttackWindow.h"
#include "Components/SekiroCombatComponent.h"
#include "GameFramework/Actor.h"

void UAnimNotifyState_AttackWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner())
		return;

	AActor* Owner = MeshComp->GetOwner();
	
	// 獲取 CombatComponent 並重置命中標記
	USekiroCombatComponent* CombatComp = Owner->FindComponentByClass<USekiroCombatComponent>();
	if (CombatComp)
	{
		CombatComp->ResetAttackHit();
	}
}

void UAnimNotifyState_AttackWindow::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner())
		return;

	AActor* Owner = MeshComp->GetOwner();
	
	// 獲取 CombatComponent
	USekiroCombatComponent* CombatComp = Owner->FindComponentByClass<USekiroCombatComponent>();
	if (CombatComp)
	{
		// 執行攻擊判定（內部會檢查是否已經命中過）
		CombatComp->PerformAttackHitCheck();
	}
}

void UAnimNotifyState_AttackWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// 攻擊窗口結束，不需要特別處理
	// bHasHit 會在下次攻擊開始時重置
}
