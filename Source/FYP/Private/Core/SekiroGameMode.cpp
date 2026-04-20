#include "Core/SekiroGameMode.h"
#include "Characters/SekiroCharacter.h"
#include "UI/SekiroHUD.h"

ASekiroGameMode::ASekiroGameMode()
{
	DefaultPawnClass = ASekiroCharacter::StaticClass();
	HUDClass = ASekiroHUD::StaticClass(); // ✅ 啟用 ASekiroHUD → 綁定「避」字 UI 事件
}
