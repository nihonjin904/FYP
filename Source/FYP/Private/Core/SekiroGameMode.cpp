#include "Core/SekiroGameMode.h"
#include "Characters/SekiroCharacter.h"

ASekiroGameMode::ASekiroGameMode()
{
	DefaultPawnClass = ASekiroCharacter::StaticClass();
}
