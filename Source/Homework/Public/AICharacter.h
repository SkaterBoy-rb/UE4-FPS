// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WeaponBaseServer.h"
#include "MultiFPSPlayerController.h"
#include "AICharacter.generated.h"

UCLASS()
class HOMEWORK_API AAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAICharacter();

	UPROPERTY(VisibleAnywhere)
	EWeaponType ActiveWeapon;

	float HP;

private:
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	UAnimInstance* ServerBodysAnimBP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	AMultiFPSPlayerController* FPSPlayerController;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerTPBodysDeadAnimMontage;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseServer* ServerPrimaryWeapon;

public:
	AWeaponBaseServer* GetCurrentServerWeapon();
	void EquipPrimary(AWeaponBaseServer* WeaponBaseServer);

	void DamagePlayer(AActor* DamageActor, AActor* DamageCauser, FVector& HitFromDirection, FHitResult& HitInfo);

	void Dead(AActor* DamageCauser);

	UFUNCTION()
	void DelayPlayDeadCallBack();

	UFUNCTION()
		void OnHit(AActor* DamagedActor, float Damage, class AController* InstigatedBy,
			FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName,
			FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable)
	void FireWeaponPrimary();

	UFUNCTION(BlueprintImplementableEvent, Category = "HP")
	void DeathMatch(AActor* DamageCauser);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void StartWithKindofWeapon();
	void PurchaseWeapon(EWeaponType WeaponType);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiShooting();
	void MultiShooting_Implementation();
	bool MultiShooting_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiDead();
	void MultiDead_Implementation();
	bool MultiDead_Validate();

};
