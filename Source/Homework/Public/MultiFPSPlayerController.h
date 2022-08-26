// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MultiFPSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class HOMEWORK_API AMultiFPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void PlayerCameraShake(TSubclassOf<UCameraShakeBase> CameraShake);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "PlyerUI")
	void CreatPlayerUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "PlyerUI")
	void DoCrosshairRecoil();

	UFUNCTION(BlueprintImplementableEvent, Category = "PlyerUI")
	void UpdateBulletUI(int32 ClipCurrBullet, int32 GunCurrBullet);

	UFUNCTION(BlueprintImplementableEvent, Category = "PlyerUI")
	void UpdateHPUI(int32 CurrHP, float Percent);

	UFUNCTION(BlueprintImplementableEvent, Category = "HP")
	void DeathMatch(AActor* DamageCauser);
};
