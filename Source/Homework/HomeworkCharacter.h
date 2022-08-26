// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Public/WeaponBaseServer.h"
#include "Public/WeaponBaseClient.h"
#include "Public/MyUserWidget.h"
#include "Public/AICharacter.h"
#include "Public/Grenade.h"
#include "Public/MultiFPSPlayerController.h"
#include "HomeworkCharacter.generated.h"

UCLASS(config=Game)
class AHomeworkCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* FP_MuzzleLocation;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* FPArmsMesh;

	bool bIsFirstPerson;

	UPROPERTY(Replicated)
	bool IsReloading;
	UPROPERTY(Replicated)
	bool IsFiring;
	UPROPERTY(Replicated)
	bool IsAiming;
	UPROPERTY(Replicated)
	bool IsExplosion;

	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	UAnimInstance* ClientArmsAnimBP;

	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	UAnimInstance* ServerBodysAnimBP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	AMultiFPSPlayerController* FPSPlayerController;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerTPBodysDeadAnimMontage_Down;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerTPBodysDeadAnimMontage_NoDown;

	UPROPERTY(Replicated, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EWeaponType ActiveWeapon;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	EWeaponType TestWeapon;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseServer* ServerPrimaryWeapon;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientPrimaryWeapon;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseServer* ServerSecondWeapon;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientSecondWeapon;

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AGrenade> Grenade;

	UPROPERTY(VisibleAnywhere, Category = SniperUI)
	UUserWidget* WidgetScope;

	UPROPERTY(EditAnywhere, Category = SniperUI)
	TSubclassOf<UUserWidget> SniperScopeBPClass;

	UPROPERTY(VisibleAnywhere, Category = SniperUI)
	UMyUserWidget* ScreenControl;

	UPROPERTY(EditAnywhere, Category = SniperUI)
	TSubclassOf<UMyUserWidget> ScreenControlBPClass;

public:
	AHomeworkCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= Character)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= Character)
	float BaseLookUpRate;

	FVector Pre_Location;
	bool IsFirstTouch;

	bool IsLockDirection;
	FRotator CurYawRotation;

	float HP;

	FTimerHandle AutoMaticFireTimeHandle;

	float NewVerticalRecoilAmout;
	float OldVerticalRecoilAmout;
	float RecoilXcoord;

	float NewHorizenRecoilAmout;
	float OldHorizenRecoilAmout;

	AGrenade* CurGrenade;

public:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	void LowSpeedWalkAction();
	void NormalSpeedWalkAction();
	void HighSpeedRunAction();

	void switchForTAction();

	void switchShootModeAction();

	void FirePressed();
	void FireReleased();

	void AimPressed();

	void Reload();

	void PitchGrenade();

	void LockDirection();
	void UnLockDirection();

public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateFPArmsBlendPose(int NewIndex);

	void EquipPrimary(AWeaponBaseServer* WeaponBaseServer);
	void EquipSecondary(AWeaponBaseServer* WeaponBaseServer);

	// ²½Ç¹Éä»÷
	void FireWeaponPrimary();
	void ReloadWeaponPrimary();
	void CSFireProcess();
	void StopFireWeaponPrimary();
	void RifleLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	// ¾Ñ»÷Ç¹Éä»÷
	void FireWeaponSniper();
	UFUNCTION()
	void StopFireWeaponSniper();
	void SniperLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	// ¼ÆÊ±Æ÷
	void AutoMaticFire();

	// ºó×øÁ¦
	void ResetRecoil();

	void DamagePlayer(AActor* DamageActor, AActor* DamageCauser, FVector& HitFromDirection, FHitResult& HitInfo);

	void Dead(AActor* DamageCauser, bool IsDown = false);

	UFUNCTION()
	void OnHit(AActor* DamagedActor, float Damage, class AController* InstigatedBy, 
		FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName,
		FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable)
	void HitedByAI(AActor* DamageCauser, float Damage);

	UFUNCTION()
	void DelayPlayArmReloadCallBack();

	UFUNCTION()
	void DelayPlayGrenadeExplosionCallBack();

	UFUNCTION()
	void DelayGetControllerCallBack();

	void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// APawn interface
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	void StartWithKindofWeapon();
	void PurchaseWeapon(EWeaponType WeaponType);

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	AWeaponBaseClient* GetCurrentClientWeapon();
	AWeaponBaseServer* GetCurrentServerWeapon();

	virtual void Tick(float DeltaTime) override;

public:
#pragma region Networking
	UFUNCTION(server, Reliable, WithValidation)
	void ServerLowSpeedWalkAction();
	void ServerLowSpeedWalkAction_Implementation();
	bool ServerLowSpeedWalkAction_Validate();

	UFUNCTION(server, Reliable, WithValidation)
	void ServerNormalSpeedWalkAction();
	void ServerNormalSpeedWalkAction_Implementation();
	bool ServerNormalSpeedWalkAction_Validate();

	UFUNCTION(server, Reliable, WithValidation)
	void ServerHighSpeedRunAction();
	void ServerHighSpeedRunAction_Implementation();
	bool ServerHighSpeedRunAction_Validate();

	UFUNCTION(server, Reliable, WithValidation)
	void ServerFireRifleWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFireRifleWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	UFUNCTION(server, Reliable, WithValidation)
	void ServerFireSniperWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireSniperWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFireSniperWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	UFUNCTION(server, Reliable, WithValidation)
	void ServerReload();
	void ServerReload_Implementation();
	bool ServerReload_Validate();

	UFUNCTION(server, Reliable, WithValidation)
	void ServerStopFire();
	void ServerStopFire_Implementation();
	bool ServerStopFire_Validate();

	UFUNCTION(server, Reliable, WithValidation)
	void ServerSetAiming();
	void ServerSetAiming_Implementation();
	bool ServerSetAiming_Validate();

	UFUNCTION(server, Reliable, WithValidation)
	void ServerGrenadeExplode();
	void ServerGrenadeExplode_Implementation();
	bool ServerGrenadeExplode_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiShooting();
	void MultiShooting_Implementation();
	bool MultiShooting_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiReload();
	void MultiReload_Implementation();
	bool MultiReload_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiDead(bool IsDown);
	void MultiDead_Implementation(bool IsDown);
	bool MultiDead_Validate(bool IsDown);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiGrenadeExplode(const FRotator SpawnRotation, const FVector SpawnLocation);
	void MultiGrenadeExplode_Implementation(const FRotator SpawnRotation, const FVector SpawnLocation);
	bool MultiGrenadeExplode_Validate(const FRotator SpawnRotation, const FVector SpawnLocation);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiSpawnBulletDecal(const FHitResult& HitInfo, FRotator Rotation, FVector ForwordVector);
	void MultiSpawnBulletDecal_Implementation(const FHitResult& HitInfo, FRotator Rotation, FVector ForwordVector);
	bool MultiSpawnBulletDecal_Validate(const FHitResult& HitInfo, FRotator Rotation, FVector ForwordVector);

	UFUNCTION(Client, Reliable)
	void ClientEquipFPArmsPrimary();
	void ClientEquipFPArmsPrimary_Implementation();

	UFUNCTION(Client, Reliable)
	void ClientEquipFPArmsSecondary();
	void ClientEquipFPArmsSecondary_Implementation();

	UFUNCTION(Client, Reliable)
	void ClientFire();
	void ClientFire_Implementation();

	UFUNCTION(Client, Reliable)
	void ClientReload();
	void ClientReload_Implementation();

	UFUNCTION(Client, Reliable)
	void ClientUpdateBulletUI(int32 ClipCurrBullet, int32 GunCurrBullet);
	void ClientUpdateBulletUI_Implementation(int32 ClipCurrBullet, int32 GunCurrBullet);

	UFUNCTION(Client, Reliable)
	void ClientUpdateHPUI(float CurrHP);
	void ClientUpdateHPUI_Implementation(float CurrHP);

	UFUNCTION(Client, Reliable)
	void ClientRecoil();
	void ClientRecoil_Implementation();

	UFUNCTION(Client, Reliable)
	void ClientAiming();
	void ClientAiming_Implementation();

	UFUNCTION(Client, Reliable)
	void ClientClearWeapon();
	void ClientClearWeapon_Implementation();
#pragma endregion
};

