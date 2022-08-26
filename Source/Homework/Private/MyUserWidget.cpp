// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUserWidget.h"
#include "../HomeworkCharacter.h"

void UMyUserWidget::SetCurrPawn(AHomeworkCharacter* APlayer)
{
	CurPlayer = APlayer;
}

void UMyUserWidget::ButtonClick(FString Name)
{
	UE_LOG(LogTemp, Warning, TEXT("FPSCharacter %s"), *Name);
	if (Name == "Fire")
		CurPlayer->FirePressed();
	else if (Name == "Jump")
		CurPlayer->Jump(); 
	else if (Name == "PitchGrenade")
		CurPlayer->PitchGrenade();
	else if (Name == "Reload")
		CurPlayer->Reload();
	else if (Name == "HighSpeedRun")
		CurPlayer->HighSpeedRunAction();
	else if (Name == "switchForT")
		CurPlayer->switchForTAction();
	else if (Name == "ALT")
		CurPlayer->LockDirection();
	else if (Name == "Aim")
		CurPlayer->AimPressed();
}

void UMyUserWidget::ButtonReleased(FString Name)
{
	if (Name == "Fire")
		CurPlayer->FireReleased();
	else if (Name == "Jump")
		CurPlayer->StopJumping();
	else if (Name == "Jump")
		CurPlayer->Jump();
	else if (Name == "HighSpeedRun")
		CurPlayer->NormalSpeedWalkAction();
	else if (Name == "ALT")
		CurPlayer->UnLockDirection();
}
