// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBaseClient.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "WeaponBaseServer.generated.h"

UENUM()
enum class EWeaponType :uint8
{
	FPS UMETA(DisplayName = "FPS"),
	Sniper UMETA(DisplayName = "Sniper")
};

UCLASS()
class HOMEWORK_API AWeaponBaseServer : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponBaseServer();
	UPROPERTY(EditAnywhere)
	EWeaponType KindOfWeapon;

	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere)
	USphereComponent* SphereCollison;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AWeaponBaseClient> ClientWeaponBaseBPClass;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere)
	USoundBase* ReloadSound;

	UPROPERTY(EditAnywhere, Replicated)
	int32 GunCurrentBullet;  // 枪里剩余子弹

	UPROPERTY(EditAnywhere, Replicated) // 如果服务器变了，客户端同步改变
	int32 ClipCurrentBullet; // 弹夹剩余子弹

	UPROPERTY(EditAnywhere)
	int32 ClipMaxBullet;	 // 弹夹容量

	UPROPERTY(EditAnywhere)
	float BulletDistance;

	UPROPERTY(EditAnywhere)
	float BaseDamage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerTPBodysShootAnimMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerTPBodysReloadAnimMontage;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* BulletDecalMaterial;

	UPROPERTY(EditAnywhere)
	UCurveFloat *VerticalRecoilCurve;

	UPROPERTY(EditAnywhere)
	UCurveFloat* HorizenRecoilCurve;

	UPROPERTY(EditAnywhere)
	bool IsAutoMatic;

	UPROPERTY(EditAnywhere)
	float AutoMaticFireRate;

	UPROPERTY(EditAnywhere)
	float MovingFireRandomRange;

	UPROPERTY(EditAnywhere)
	float Impulse;

	UFUNCTION()
		void OnOtherBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor,
			class UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void EquipWeapon(bool IsPlayer = true);

	void SetVisible(bool IsVisible);

	void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiShootingEffect();
	void MultiShootingEffect_Implementation();
	bool MultiShootingEffect_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiReloadEffect();
	void MultiReloadEffect_Implementation();
	bool MultiReloadEffect_Validate();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
