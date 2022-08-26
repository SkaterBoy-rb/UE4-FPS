// Copyright Epic Games, Inc. All Rights Reserved.

#include "HomeworkGameMode.h"
#include "HomeworkCharacter.h"
#include "public/MultiFPSPlayerController.h"
#include "UObject/ConstructorHelpers.h"

AHomeworkGameMode::AHomeworkGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BluePrint/Player/BP_Attack"));
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/BluePrint/Game/BP_MultiFPSPlayerController"));
	if (PlayerControllerBPClass.Class)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}
