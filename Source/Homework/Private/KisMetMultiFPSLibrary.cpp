// Fill out your copyright notice in the Description page of Project Settings.


#include "KisMetMultiFPSLibrary.h"

void UKisMetMultiFPSLibrary::SortValues(TArray<FDeathMatchPlayerData>& Values)
{
	Values.Sort(
		[](const FDeathMatchPlayerData& a, const FDeathMatchPlayerData& b) ->bool
		{
			return a.PlayerScore > b.PlayerScore;
		});
}
