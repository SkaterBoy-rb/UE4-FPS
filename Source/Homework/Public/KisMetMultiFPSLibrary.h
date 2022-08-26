// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "KisMetMultiFPSLibrary.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FDeathMatchPlayerData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName PlayerName;
	UPROPERTY(BlueprintReadWrite)
	int PlayerScore;

	FDeathMatchPlayerData()
	:PlayerName(TEXT("")),
	 PlayerScore(0)
	{}
};

UCLASS()
class HOMEWORK_API UKisMetMultiFPSLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Sort")
	static void SortValues(UPARAM(ref)TArray<FDeathMatchPlayerData>& Values);
	
};
