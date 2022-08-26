// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyUserWidget.generated.h"

class AHomeworkCharacter;
/**
 * 
 */
UCLASS()
class HOMEWORK_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetCurrPawn(AHomeworkCharacter* APlayer);

	UFUNCTION(BlueprintCallable)
	void ButtonClick(FString Name);
	UFUNCTION(BlueprintCallable)
	void ButtonReleased(FString Name);


private:
	AHomeworkCharacter* CurPlayer;
};
