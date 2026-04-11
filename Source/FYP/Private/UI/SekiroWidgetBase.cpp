#include "UI/SekiroWidgetBase.h"
#include "Components/WidgetComponent.h"
#include "Components/SekiroAttributeComponent.h"
#include "Components/SekiroPostureComponent.h"
#include "Engine/Engine.h"

void USekiroWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	// 透過 WidgetComponent 找到 Owner Actor
	UWidgetComponent* WidgetComp = Cast<UWidgetComponent>(GetOuter());
	if (!WidgetComp)
	{
		// 嘗試另一個方法：透過 GetTypedOuter 往上找
		WidgetComp = GetTypedOuter<UWidgetComponent>();
	}
	if (!WidgetComp)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange,
			FString::Printf(TEXT("[SekiroWidgetBase] GetOuter()=%s — NOT a WidgetComponent, skipping"),
				GetOuter() ? *GetOuter()->GetClass()->GetName() : TEXT("NULL")));
		return;
	}

	AActor* Owner = WidgetComp->GetOwner();
	if (!Owner)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
			TEXT("[SekiroWidgetBase] WidgetComp found but Owner is NULL!"));
		return;
	}

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan,
		FString::Printf(TEXT("[SekiroWidgetBase] Owner=%s, binding..."), *Owner->GetName()));

	// 綁定血條 —— 找 Owner 身上的 USekiroAttributeComponent
	USekiroAttributeComponent* AttrComp = Owner->FindComponentByClass<USekiroAttributeComponent>();
	if (AttrComp)
	{
		AttrComp->OnHealthChanged.AddDynamic(this, &USekiroWidgetBase::HandleHealthChanged);
		UpdateHealth(AttrComp->CurrentHealth, AttrComp->MaxHealth);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
			FString::Printf(TEXT("[SekiroWidgetBase] Health bound! HP=%.0f/%.0f"), AttrComp->CurrentHealth, AttrComp->MaxHealth));
	} else {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
			TEXT("[SekiroWidgetBase] NO SekiroAttributeComponent found on Owner!"));
	}

	// 綁定架勢 —— 找 Owner 身上的 USekiroPostureComponent
	USekiroPostureComponent* PostureComp = Owner->FindComponentByClass<USekiroPostureComponent>();
	if (PostureComp)
	{
		PostureComp->OnPostureChanged.AddDynamic(this, &USekiroWidgetBase::HandlePostureChanged);
		UpdateEnemyPosture(PostureComp->CurrentPosture, PostureComp->MaxPosture);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
			FString::Printf(TEXT("[SekiroWidgetBase] Posture bound! P=%.0f/%.0f"), PostureComp->CurrentPosture, PostureComp->MaxPosture));
	} else {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
			TEXT("[SekiroWidgetBase] NO SekiroPostureComponent found on Owner!"));
	}
}

void USekiroWidgetBase::HandleHealthChanged(float NewHealth, float MaxHealth)
{
	UpdateHealth(NewHealth, MaxHealth);
}

void USekiroWidgetBase::HandlePostureChanged(float CurrentPosture, float MaxPosture)
{
	UpdateEnemyPosture(CurrentPosture, MaxPosture);
}
