// Copyright Epic Games, Inc. All Rights Reserved.

#include "HomeworkCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// AHomeworkCharacter
const TMap<EWeaponType, FName> ArmLocation = {
	{EWeaponType::FPS, TEXT("weapon_socket_AK47")},
	{EWeaponType::Sniper, TEXT("weapon_socket_Sniper")}
};

const TMap<EWeaponType, FName> BodyLocation = {
	{EWeaponType::FPS, TEXT("Weapon_FPS")},
	{EWeaponType::Sniper, TEXT("Weapon_Sniper")}
};

AHomeworkCharacter::AHomeworkCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a SkeletalMesh
	FPArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPArmsMesh"));
	if (FPArmsMesh)
	{
		FPArmsMesh->SetupAttachment(FollowCamera);
		FPArmsMesh->SetOnlyOwnerSee(true);
	}

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FPArmsMesh);

	GetMesh()->SetOnlyOwnerSee(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AHomeworkCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("PitchGrenade", IE_Pressed, this, &AHomeworkCharacter::PitchGrenade);
	PlayerInputComponent->BindAction("switchShootMode", IE_Pressed, this, &AHomeworkCharacter::switchShootModeAction);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AHomeworkCharacter::FirePressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AHomeworkCharacter::FireReleased);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AHomeworkCharacter::AimPressed);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AHomeworkCharacter::Reload);
	PlayerInputComponent->BindAction("LowSpeedWalk", IE_Pressed, this, &AHomeworkCharacter::LowSpeedWalkAction);
	PlayerInputComponent->BindAction("LowSpeedWalk", IE_Released, this, &AHomeworkCharacter::NormalSpeedWalkAction);
	PlayerInputComponent->BindAction("HighSpeedRun", IE_Pressed, this, &AHomeworkCharacter::HighSpeedRunAction);
	PlayerInputComponent->BindAction("HighSpeedRun", IE_Released, this, &AHomeworkCharacter::NormalSpeedWalkAction);
	PlayerInputComponent->BindAction("switchForT", IE_Pressed, this, &AHomeworkCharacter::switchForTAction);
	PlayerInputComponent->BindAction("LockDirection", IE_Pressed, this, &AHomeworkCharacter::LockDirection);
	PlayerInputComponent->BindAction("LockDirection", IE_Released, this, &AHomeworkCharacter::UnLockDirection);

	PlayerInputComponent->BindAxis("MoveForward", this, &AHomeworkCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHomeworkCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	//PlayerInputComponent->BindAxis("TurnRate", this, &AHomeworkCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	//PlayerInputComponent->BindAxis("LookUpRate", this, &AHomeworkCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Repeat, this, &AHomeworkCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AHomeworkCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AHomeworkCharacter::OnResetVR);
}


void AHomeworkCharacter::StartWithKindofWeapon()
{
	/*UKismetSystemLibrary::PrintString(this,
		FString::Printf(TEXT("HasAuthority:%d"), HasAuthority() ? 1 : 0));*/
	if (HasAuthority())
	{
		PurchaseWeapon(TestWeapon);
	}
}

void AHomeworkCharacter::PurchaseWeapon(EWeaponType WeaponType)
{
	FActorSpawnParameters SapwnInfo;
	SapwnInfo.Owner = this;
	SapwnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	switch (WeaponType)
	{
	case EWeaponType::FPS:
	{
		UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr,
			TEXT("BluePrint'/Game/BluePrint/weapon/FPS/ServerBP_FPS.ServerBP_FPS_C'"));
		AWeaponBaseServer* ServerWeapon =
			GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SapwnInfo);
		ServerWeapon->EquipWeapon();
		EquipPrimary(ServerWeapon);
		break;
	}
	case EWeaponType::Sniper:
	{
		UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr,
			TEXT("BluePrint'/Game/BluePrint/weapon/Sniper/ServerBP_Sniper.ServerBP_Sniper_C'"));
		AWeaponBaseServer* ServerWeapon =
			GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SapwnInfo);
		ServerWeapon->EquipWeapon();
		EquipPrimary(ServerWeapon);
		break;
	}
	default:
		break;
	}
}

AWeaponBaseClient* AHomeworkCharacter::GetCurrentClientWeapon()
{
	switch (ActiveWeapon)
	{
	case EWeaponType::FPS:
	case EWeaponType::Sniper:
	{
		return ClientPrimaryWeapon;
	}
	default:
		return nullptr;
	}
}

AWeaponBaseServer* AHomeworkCharacter::GetCurrentServerWeapon()
{
	switch (ActiveWeapon)
	{
	case EWeaponType::FPS:
	case EWeaponType::Sniper:
	{
		return ServerPrimaryWeapon;
	}
	default:
		return nullptr;
	}
}

void AHomeworkCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HP == 0)
		Dead(nullptr, true);
}

#pragma region Networking
void AHomeworkCharacter::ServerLowSpeedWalkAction_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 300;
}

bool AHomeworkCharacter::ServerLowSpeedWalkAction_Validate()
{
	return true;
}

void AHomeworkCharacter::ServerNormalSpeedWalkAction_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 600;
}

bool AHomeworkCharacter::ServerNormalSpeedWalkAction_Validate()
{
	return true;
}

void AHomeworkCharacter::ServerHighSpeedRunAction_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 1200;
}

bool AHomeworkCharacter::ServerHighSpeedRunAction_Validate()
{
	return true;
}

void AHomeworkCharacter::ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	if (ServerPrimaryWeapon)
	{
		// 多播射击特效
		ServerPrimaryWeapon->MultiShootingEffect();
		ServerPrimaryWeapon->ClipCurrentBullet -= 1;
		// 多播身体动画
		MultiShooting();

		ClientUpdateBulletUI(ServerPrimaryWeapon->ClipCurrentBullet,
			ServerPrimaryWeapon->GunCurrentBullet);

		RifleLineTrace(CameraLocation, CameraRotation, IsMoving);
		IsFiring = true;
	}
}

bool AHomeworkCharacter::ServerFireRifleWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void AHomeworkCharacter::ServerFireSniperWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	AWeaponBaseServer* CurServerWeapon = GetCurrentServerWeapon();
	if (CurServerWeapon)
	{
		// 多播射击特效
		CurServerWeapon->MultiShootingEffect();
		CurServerWeapon->ClipCurrentBullet -= 1;
		// 多播身体动画
		MultiShooting();

		ClientUpdateBulletUI(CurServerWeapon->ClipCurrentBullet,
			CurServerWeapon->GunCurrentBullet);
		AWeaponBaseClient* CurClientWeapon = GetCurrentClientWeapon();
		if (CurClientWeapon)
		{
			UE_LOG(LogTemp, Warning, TEXT("CurClientWeapon"));
			FLatentActionInfo ActionInfo(0, FMath::Rand(), TEXT("StopFireWeaponSniper"), this);
			UKismetSystemLibrary::Delay(this,
				CurClientWeapon->ClientArmsFireMontage->GetPlayLength(), ActionInfo);
		}
		SniperLineTrace(CameraLocation, CameraRotation, IsMoving);
		IsFiring = true;
	}
}

bool AHomeworkCharacter::ServerFireSniperWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void AHomeworkCharacter::ServerReload_Implementation()
{
	// 多播身体动画
	GetCurrentServerWeapon()->MultiReloadEffect();
	MultiReload();
	IsReloading = true;
	AWeaponBaseClient* CurClientWeapon = GetCurrentClientWeapon();
	if (CurClientWeapon)
	{
		FLatentActionInfo ActionInfo(0, FMath::Rand(), TEXT("DelayPlayArmReloadCallBack"), this);
		UKismetSystemLibrary::Delay(this,
			CurClientWeapon->ClientArmsReloadMontage->GetPlayLength(), ActionInfo);
	}
}

bool AHomeworkCharacter::ServerReload_Validate()
{
	return true;
}

void AHomeworkCharacter::ServerStopFire_Implementation()
{
	IsFiring = false;
}

bool AHomeworkCharacter::ServerStopFire_Validate()
{
	return true;
}

void AHomeworkCharacter::ServerSetAiming_Implementation()
{
	IsAiming = !IsAiming;
}

bool AHomeworkCharacter::ServerSetAiming_Validate()
{
	return true;
}

void AHomeworkCharacter::ServerGrenadeExplode_Implementation()
{
	//  解决位置问题
	const FRotator SpawnRotation = GetControlRotation();
	// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
	const FVector SpawnLocation = FP_MuzzleLocation->GetComponentLocation();
	IsExplosion = true;
	MultiGrenadeExplode(SpawnRotation, SpawnLocation);
}

bool AHomeworkCharacter::ServerGrenadeExplode_Validate()
{
	return true;
}

void AHomeworkCharacter::MultiShooting_Implementation()
{
	if (ServerBodysAnimBP)
	{
		AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerWeapon();
		if (CurrentServerWeapon)
		{
			ServerBodysAnimBP->Montage_Play(CurrentServerWeapon->ServerTPBodysShootAnimMontage);
		}
	}
}

bool AHomeworkCharacter::MultiShooting_Validate()
{
	return true;
}

void AHomeworkCharacter::MultiReload_Implementation()
{
	if (ServerBodysAnimBP)
	{
		AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerWeapon();
		if (CurrentServerWeapon)
		{
			ServerBodysAnimBP->Montage_Play(CurrentServerWeapon->ServerTPBodysReloadAnimMontage);
		}
	}
}

bool AHomeworkCharacter::MultiReload_Validate()
{
	return true;
}

void AHomeworkCharacter::MultiDead_Implementation(bool IsDown)
{
	if (ServerBodysAnimBP)
	{
		if (IsDown)
			ServerBodysAnimBP->Montage_Play(ServerTPBodysDeadAnimMontage_Down);
		else
			ServerBodysAnimBP->Montage_Play(ServerTPBodysDeadAnimMontage_NoDown);
	}
}

bool AHomeworkCharacter::MultiDead_Validate(bool IsDown)
{
	return true;
}

void AHomeworkCharacter::MultiGrenadeExplode_Implementation(const FRotator SpawnRotation, const FVector SpawnLocation)
{
	if (Grenade != nullptr)
	{
		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// spawn the projectile at the muzzle
		CurGrenade = GetWorld()->SpawnActor<AGrenade>(Grenade, SpawnLocation, SpawnRotation, ActorSpawnParams);
		FLatentActionInfo ActionInfo(0, FMath::Rand(), TEXT("DelayPlayGrenadeExplosionCallBack"), this);
		UKismetSystemLibrary::Delay(this, 3.0f, ActionInfo);
	}
}

bool AHomeworkCharacter::MultiGrenadeExplode_Validate(const FRotator SpawnRotation, const FVector SpawnLocation)
{
	return true;
}

void AHomeworkCharacter::MultiSpawnBulletDecal_Implementation(const FHitResult& HitInfo, FRotator Rotation,
	FVector ForwordVector)
{
	AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerWeapon();
	if (CurrentServerWeapon)
	{
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CurrentServerWeapon->BulletDecalMaterial,
			FVector(8, 8, 8), HitInfo.Location, Rotation, 10);
		if (Decal)
		{
			Decal->SetFadeScreenSize(0.001);
		}
	}
	// 同时对物理模拟的物体施加冲力
	if (((HitInfo.Actor).Get() != nullptr)  && ((HitInfo.Component).Get() != nullptr) 
		&& (HitInfo.Component).Get()->IsSimulatingPhysics())
	{
		(HitInfo.Component).Get()->AddImpulseAtLocation(ForwordVector * 
			CurrentServerWeapon->Impulse, (HitInfo.Actor).Get()->GetActorLocation());
	}
}

bool AHomeworkCharacter::MultiSpawnBulletDecal_Validate(const FHitResult& HitInfo, FRotator Rotation,
	FVector ForwordVector)
{
	return true;
}

void AHomeworkCharacter::ClientEquipFPArmsPrimary_Implementation()
{
	if (ServerPrimaryWeapon)
	{
		if (!ClientPrimaryWeapon)
		{
			FActorSpawnParameters SapwnInfo;
			SapwnInfo.Owner = this;
			SapwnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ClientPrimaryWeapon =
				GetWorld()->SpawnActor<AWeaponBaseClient>(ServerPrimaryWeapon->ClientWeaponBaseBPClass,
					GetActorTransform(), SapwnInfo);
			if (bIsFirstPerson)
				ClientPrimaryWeapon->K2_AttachToComponent(FPArmsMesh, ArmLocation[ActiveWeapon],
					EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
			else
				ClientPrimaryWeapon->K2_AttachToComponent(GetMesh(), BodyLocation[ActiveWeapon],
					EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
			ClientUpdateBulletUI(ServerPrimaryWeapon->ClipCurrentBullet, ServerPrimaryWeapon->GunCurrentBullet);
		}
	}
}

void AHomeworkCharacter::ClientEquipFPArmsSecondary_Implementation()
{
	if (ServerSecondWeapon)
	{
		if (!ClientSecondWeapon)
		{
			FActorSpawnParameters SapwnInfo;
			SapwnInfo.Owner = this;
			SapwnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ClientSecondWeapon =
				GetWorld()->SpawnActor<AWeaponBaseClient>(ServerSecondWeapon->ClientWeaponBaseBPClass,
					GetActorTransform(), SapwnInfo);
			if (bIsFirstPerson)
				ClientSecondWeapon->K2_AttachToComponent(FPArmsMesh, ArmLocation[ActiveWeapon],
					EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
			else
				ClientSecondWeapon->K2_AttachToComponent(GetMesh(), BodyLocation[ActiveWeapon],
					EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
			//ClientUpdateBulletUI(ServerSecondWeapon->ClipCurrentBullet, ServerSecondWeapon->GunCurrentBullet);
		}
	}
}

void AHomeworkCharacter::ClientFire_Implementation()
{
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientWeapon();
	if (CurrentClientWeapon)
	{
		// 枪体播放动画
		CurrentClientWeapon->PlayShootAnimation();
		//手臂播放动画
		UAnimMontage* ClientArmsFireMontage = CurrentClientWeapon->ClientArmsFireMontage;
		ClientArmsAnimBP->Montage_SetPlayRate(ClientArmsFireMontage, 1);
		ClientArmsAnimBP->Montage_Play(ClientArmsFireMontage);

		//播放射击声音
		CurrentClientWeapon->DisplayWeaponEffect();
		if (FPSPlayerController)
		{
			//应用屏幕抖动
			FPSPlayerController->PlayerCameraShake(CurrentClientWeapon->CameraShakeClass);

			//准星扩散动画
			FPSPlayerController->DoCrosshairRecoil();
		}
	}
}

void AHomeworkCharacter::ClientReload_Implementation()
{
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientWeapon();
	if (CurrentClientWeapon)
	{
		// 枪体播放动画
		CurrentClientWeapon->PlayReloadAnimation();
		//手臂播放动画
		UAnimMontage* ClientArmsReloadMontage = CurrentClientWeapon->ClientArmsReloadMontage;
		ClientArmsAnimBP->Montage_SetPlayRate(ClientArmsReloadMontage, 1);
		ClientArmsAnimBP->Montage_Play(ClientArmsReloadMontage);

		//播放换弹声音
		UGameplayStatics::PlaySound2D(GetWorld(), CurrentClientWeapon->ReloadSound);
	}
}

void AHomeworkCharacter::ClientUpdateBulletUI_Implementation(int32 ClipCurrBullet, int32 GunCurrBullet)
{
	if (FPSPlayerController)
	{
		FPSPlayerController->UpdateBulletUI(ClipCurrBullet, GunCurrBullet);
	}
}

void AHomeworkCharacter::ClientUpdateHPUI_Implementation(float CurrHP)
{
	if (FPSPlayerController)
	{
		FPSPlayerController->UpdateHPUI(int32(CurrHP), CurrHP / 100.0);
	}
}

void AHomeworkCharacter::ClientRecoil_Implementation()
{
	UCurveFloat* VerticalRecoilCurve = nullptr;
	UCurveFloat* HorizenRecoilCurve = nullptr;
	VerticalRecoilCurve = GetCurrentServerWeapon()->VerticalRecoilCurve;
	HorizenRecoilCurve = GetCurrentServerWeapon()->HorizenRecoilCurve;
	RecoilXcoord += 0.1;
	if (VerticalRecoilCurve)
		NewVerticalRecoilAmout = VerticalRecoilCurve->GetFloatValue(RecoilXcoord);
	if (HorizenRecoilCurve)
		NewHorizenRecoilAmout = HorizenRecoilCurve->GetFloatValue(RecoilXcoord);
	float dif_amountVertical = NewVerticalRecoilAmout - OldVerticalRecoilAmout;
	float dif_amountHorizen = NewHorizenRecoilAmout - OldHorizenRecoilAmout;
	if (FPSPlayerController)
	{
		FRotator ControllerRotator = FPSPlayerController->GetControlRotation();
		FPSPlayerController->SetControlRotation(FRotator(ControllerRotator.Pitch +
			dif_amountVertical, ControllerRotator.Yaw + dif_amountHorizen,
			ControllerRotator.Roll));
	}
	OldVerticalRecoilAmout = NewVerticalRecoilAmout;
	OldHorizenRecoilAmout = NewHorizenRecoilAmout;
}

void AHomeworkCharacter::ClientAiming_Implementation()
{
	if (bIsFirstPerson)
	{
		FPArmsMesh->SetHiddenInGame(!IsAiming);
	}
	else
		GetMesh()->SetHiddenInGame(!IsAiming);
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientWeapon();
	if (!IsAiming)
	{
		if (CurrentClientWeapon)
		{
			CurrentClientWeapon->SetActorHiddenInGame(!IsAiming);
			FollowCamera->SetFieldOfView(CurrentClientWeapon->FieldOfAimingView);
		}
		WidgetScope = CreateWidget<UUserWidget>(GetWorld(), SniperScopeBPClass);
		ScreenControl->RemoveFromParent();
		WidgetScope->AddToViewport();
		ScreenControl->AddToViewport();
	}
	else
	{
		if (CurrentClientWeapon)
		{
			CurrentClientWeapon->SetActorHiddenInGame(!IsAiming);
			FollowCamera->SetFieldOfView(90);
		}
		if (WidgetScope)
		{
			WidgetScope->RemoveFromParent();
		}
	}
}

void AHomeworkCharacter::ClientClearWeapon_Implementation()
{
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientWeapon();
	if (CurrentClientWeapon)
	{
		CurrentClientWeapon->Destroy();
		CurrentClientWeapon = nullptr;
	}
}

#pragma endregion

void AHomeworkCharacter::OnResetVR()
{
	// If Homework is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in Homework.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AHomeworkCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	/*UKismetSystemLibrary::PrintString(this,
		FString::Printf(TEXT("S_Location = %f %f %f"), Location.X, Location.Y, Location.Z));*/
	if (IsFirstTouch)
	{
		IsFirstTouch = false;
		Pre_Location = Location;
	}
	TurnAtRate(Location.X - Pre_Location.X);
	LookUpAtRate(Location.Y - Pre_Location.Y);

	Pre_Location = Location;
}

void AHomeworkCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	IsFirstTouch = true;
}

void AHomeworkCharacter::LowSpeedWalkAction()
{
	GetCharacterMovement()->MaxWalkSpeed = 300;
	ServerLowSpeedWalkAction();
	//UE_LOG(LogTemp, Warning, TEXT("cur dis = %f"), CameraBoom->TargetArmLength);
}

void AHomeworkCharacter::NormalSpeedWalkAction()
{
	GetCharacterMovement()->MaxWalkSpeed = 600;
	ServerNormalSpeedWalkAction();
}

void AHomeworkCharacter::HighSpeedRunAction()
{
	GetCharacterMovement()->MaxWalkSpeed = 1200;
	ServerHighSpeedRunAction();
}

void AHomeworkCharacter::switchForTAction()
{
	if (!IsAiming)
	{
		CameraBoom->TargetArmLength = bIsFirstPerson ? 600.0f : 300.0f;
		GetMesh()->SetOwnerNoSee(!bIsFirstPerson);
		FPArmsMesh->SetOwnerNoSee(bIsFirstPerson);
		bIsFirstPerson = !bIsFirstPerson;
		AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientWeapon();
		if (CurrentClientWeapon)
		{
			if (bIsFirstPerson)
				CurrentClientWeapon->K2_AttachToComponent(FPArmsMesh, ArmLocation[ActiveWeapon],
					EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
			else
				CurrentClientWeapon->K2_AttachToComponent(GetMesh(), BodyLocation[ActiveWeapon],
					EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
		}
	}
}

void AHomeworkCharacter::switchShootModeAction()
{
	AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerWeapon();
	if (CurrentServerWeapon)
	{
		CurrentServerWeapon->IsAutoMatic = !CurrentServerWeapon->IsAutoMatic;
	}
}

void AHomeworkCharacter::EquipPrimary(AWeaponBaseServer* WeaponBaseServer)
{
	/*UKismetSystemLibrary::PrintString(this,
		FString::Printf(TEXT("EquipPrimary : %d"), ServerPrimaryWeapon ? 1 : 0));*/
	if (!ServerPrimaryWeapon)
	{
		ServerPrimaryWeapon = WeaponBaseServer;
		ServerPrimaryWeapon->SetOwner(this);
		ActiveWeapon = ServerPrimaryWeapon->KindOfWeapon;
		ServerPrimaryWeapon->K2_AttachToComponent(GetMesh(), BodyLocation[ActiveWeapon],
			EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
		ClientEquipFPArmsPrimary();
		// 手臂动画混合
		UpdateFPArmsBlendPose(ClientPrimaryWeapon->BlendPose);
	}
	else
	{

	}
	
}

void AHomeworkCharacter::EquipSecondary(AWeaponBaseServer* WeaponBaseServer)
{
	if (!ServerSecondWeapon)
	{
		ServerSecondWeapon = WeaponBaseServer;
		ServerSecondWeapon->SetOwner(this);
		ServerSecondWeapon->K2_AttachToComponent(GetMesh(), BodyLocation[ServerSecondWeapon->KindOfWeapon],
			EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
		ClientEquipFPArmsSecondary();
		ServerSecondWeapon->SetVisible(false);
		ClientSecondWeapon->SetVisible(false);
	}
}

void AHomeworkCharacter::FireWeaponPrimary()
{
	/*UKismetSystemLibrary::PrintString(this,
		FString::Printf(TEXT("FireWeaponPrimary:%d"), ServerPrimaryWeapon ? 1 : 0));*/
	AWeaponBaseServer* CurServerWeapon = GetCurrentServerWeapon();
	if (CurServerWeapon->ClipCurrentBullet > 0 && !IsReloading)
	{
		CSFireProcess();
		// 全自动
		if (CurServerWeapon->IsAutoMatic)
		{
			GetWorldTimerManager().SetTimer(AutoMaticFireTimeHandle, this,
				&AHomeworkCharacter::AutoMaticFire, CurServerWeapon->AutoMaticFireRate, true);
		}
	}

}

void AHomeworkCharacter::ReloadWeaponPrimary()
{
	AWeaponBaseServer* CurServerWeapon = GetCurrentServerWeapon();
	if (CurServerWeapon->ClipCurrentBullet != CurServerWeapon->ClipMaxBullet
		&& CurServerWeapon->GunCurrentBullet > 0)
	{
		// 客户端换弹
		ClientReload();
		ServerReload();
		if (IsAiming)
		{
			ServerSetAiming();
			ClientAiming();
		}
	}
}

void AHomeworkCharacter::CSFireProcess()
{
	// 服务端
	bool IsMoving = false;
	if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		IsMoving = true;
	if (ActiveWeapon != EWeaponType::Sniper)
		ServerFireRifleWeapon(FollowCamera->GetComponentLocation(),
			FollowCamera->GetComponentRotation(), IsMoving);
	else
		ServerFireSniperWeapon(FollowCamera->GetComponentLocation(),
			FollowCamera->GetComponentRotation(), IsMoving);
	/*UE_LOG(LogTemp, Warning, TEXT("FireWeaponPrimary"));
	UKismetSystemLibrary::PrintString(this,
		FString::Printf(TEXT(":%d"), ServerPrimaryWeapon->ClipCurrentBullet));*/
		// 客户端
	ClientFire();
}

void AHomeworkCharacter::StopFireWeaponPrimary()
{
	GetWorldTimerManager().ClearTimer(AutoMaticFireTimeHandle);
	// 重置后坐力相关
	ResetRecoil();

	ServerStopFire();
}

void AHomeworkCharacter::RifleLineTrace(FVector CameraLocation, FRotator CameraRotation,
	bool IsMoving)
{
	AWeaponBaseServer* CurServerWeapon = GetCurrentServerWeapon();
	if (CurServerWeapon)
	{
		FVector EndLocation;
		FVector CameraForwardVector = UKismetMathLibrary::GetForwardVector(CameraRotation);
		TArray<AActor*> IgnoreArray;
		IgnoreArray.Add(this);
		FHitResult HitResult;
		if (IsMoving)
		{
			FVector Fvec = CameraLocation + CameraForwardVector * CurServerWeapon->BulletDistance;
			float RandomX = UKismetMathLibrary::RandomFloatInRange(
				-CurServerWeapon->MovingFireRandomRange,
				CurServerWeapon->MovingFireRandomRange);
			float RandomY = UKismetMathLibrary::RandomFloatInRange(
				-CurServerWeapon->MovingFireRandomRange,
				CurServerWeapon->MovingFireRandomRange);
			float RandomZ = UKismetMathLibrary::RandomFloatInRange(
				-CurServerWeapon->MovingFireRandomRange,
				CurServerWeapon->MovingFireRandomRange);
			EndLocation = FVector(Fvec.X + RandomX, Fvec.Y + RandomY, Fvec.Z + RandomZ);
		}
		else
		{
			EndLocation = CameraLocation + CameraForwardVector * CurServerWeapon->BulletDistance;
		}
		bool HitSuccess = UKismetSystemLibrary::LineTraceSingle(GetWorld(), CameraLocation, EndLocation,
			ETraceTypeQuery::TraceTypeQuery1, false, IgnoreArray, EDrawDebugTrace::None,
			HitResult, true);
		if (HitSuccess)
		{
			UKismetSystemLibrary::PrintString(GetWorld(),
				FString::Printf(TEXT("Hit Actor is :%s"), *(HitResult.Actor->GetName())));
			if ((HitResult.Actor).Get()->IsA(AHomeworkCharacter::StaticClass())
				|| (HitResult.Actor).Get()->IsA(AAICharacter::StaticClass()))
			{
				// 达到玩家，应用伤害
				DamagePlayer((HitResult.Actor).Get(), this, CameraLocation, HitResult);
			}
			else
			{
				// 打到墙壁，生成弹孔
				FRotator XRotator = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
				MultiSpawnBulletDecal(HitResult, XRotator, CameraForwardVector);
			}
		}
	}
}

void AHomeworkCharacter::FireWeaponSniper()
{
	if (GetCurrentServerWeapon()->ClipCurrentBullet > 0 && !IsReloading && !IsFiring)
	{
		CSFireProcess();
	}
}

void AHomeworkCharacter::StopFireWeaponSniper()
{
	IsFiring = false;
}

void AHomeworkCharacter::SniperLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	AWeaponBaseServer* CurServerWeapon = GetCurrentServerWeapon();
	if (CurServerWeapon)
	{
		FVector EndLocation;
		FVector CameraForwardVector = UKismetMathLibrary::GetForwardVector(CameraRotation);
		TArray<AActor*> IgnoreArray;
		IgnoreArray.Add(this);
		FHitResult HitResult;
		// 是否开镜导致不同的射线检测
		if (IsMoving || !IsAiming)
		{
			FVector Fvec = CameraLocation + CameraForwardVector * CurServerWeapon->BulletDistance;
			float RandomX = UKismetMathLibrary::RandomFloatInRange(
				-CurServerWeapon->MovingFireRandomRange,
				CurServerWeapon->MovingFireRandomRange);
			float RandomY = UKismetMathLibrary::RandomFloatInRange(
				-CurServerWeapon->MovingFireRandomRange,
				CurServerWeapon->MovingFireRandomRange);
			float RandomZ = UKismetMathLibrary::RandomFloatInRange(
				-CurServerWeapon->MovingFireRandomRange,
				CurServerWeapon->MovingFireRandomRange);
			EndLocation = FVector(Fvec.X + RandomX, Fvec.Y + RandomY, Fvec.Z + RandomZ);
		}
		else
		{
			EndLocation = CameraLocation + CameraForwardVector * CurServerWeapon->BulletDistance;
		}
		if (IsAiming)
		{
			ServerSetAiming();
			ClientAiming();
		}
		bool HitSuccess = UKismetSystemLibrary::LineTraceSingle(GetWorld(), CameraLocation, EndLocation,
			ETraceTypeQuery::TraceTypeQuery1, false, IgnoreArray, EDrawDebugTrace::None,
			HitResult, true);
		if (HitSuccess)
		{
			UKismetSystemLibrary::PrintString(GetWorld(),
				FString::Printf(TEXT("Hit Actor is :%s"), *(HitResult.Actor->GetName())));
			if ((HitResult.Actor).Get()->IsA(AHomeworkCharacter::StaticClass())
				|| (HitResult.Actor).Get()->IsA(AAICharacter::StaticClass()))
			{
				// 达到玩家，应用伤害
				DamagePlayer((HitResult.Actor).Get(), this, CameraLocation, HitResult);
			}
			else
			{
				// 打到墙壁，生成弹孔
				FRotator XRotator = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
				MultiSpawnBulletDecal(HitResult, XRotator, CameraForwardVector);
			}
		}
	}
}

void AHomeworkCharacter::AutoMaticFire()
{
	if (GetCurrentServerWeapon()->ClipCurrentBullet > 0)
	{
		CSFireProcess();
		ClientRecoil();
	}
	else
	{
		StopFireWeaponPrimary();
	}
}

void AHomeworkCharacter::ResetRecoil()
{
	RecoilXcoord = 0;
	OldVerticalRecoilAmout = 0;
	NewVerticalRecoilAmout = 0;
	OldHorizenRecoilAmout = 0;
	NewHorizenRecoilAmout = 0;
}

void AHomeworkCharacter::DamagePlayer(AActor* DamageActor, AActor* DamageCauser, FVector& HitFromDirection,
	FHitResult& HitInfo)
{
	AHomeworkCharacter* HWCharactor = Cast<AHomeworkCharacter>(DamageCauser);
	float Damage;
	if (HWCharactor)
	{
		Damage = HWCharactor->GetCurrentServerWeapon()->BaseDamage;
		UPhysicalMaterial* PhysicalMaterial = (HitInfo.PhysMaterial).Get();
		
		switch (PhysicalMaterial->SurfaceType)
		{
		case EPhysicalSurface::SurfaceType1:
		{
			// head
			Damage *= 4;
			break;
		}
		case EPhysicalSurface::SurfaceType2:
		{
			// body
			Damage *= 1;
			break;
		}
		case EPhysicalSurface::SurfaceType3:
		{
			// arm
			Damage *= 0.8;
			break;
		}
		case EPhysicalSurface::SurfaceType4:
		{
			// leg
			Damage *= 0.7;
			break;
		}
		}
	}
	else
	{
		// 投掷物（grenade）
		float Distance = pow(DamageCauser->GetActorLocation().X - DamageActor->GetActorLocation().X, 2)
			+ pow(DamageCauser->GetActorLocation().Y - DamageActor->GetActorLocation().Y, 2);
		if (Distance < 160000)
			Damage = 100;
		else if (Distance < 1000000)
			Damage = 50;
		else
			Damage = 20;
	}
	// 底层观察者模式,打了别人发通知
	UGameplayStatics::ApplyPointDamage(DamageActor, Damage, HitFromDirection,
		HitInfo, GetController(), DamageCauser, UDamageType::StaticClass());
	// 当自己被打时的回调OnHit
}

void AHomeworkCharacter::Dead(AActor* DamageCauser, bool IsDown)
{
	// 服务端
	MultiDead(IsDown);
	if (ClientPrimaryWeapon)
	{
		ClientPrimaryWeapon->Destroy();
		ClientPrimaryWeapon = nullptr;
	}
	if (ServerPrimaryWeapon)
	{
		ServerPrimaryWeapon->Destroy();
		ServerPrimaryWeapon = nullptr;
	}
	if (ClientSecondWeapon)
		ClientSecondWeapon->Destroy();
	if (ServerSecondWeapon)
		ServerSecondWeapon->Destroy();
	// 客户端
	ClientClearWeapon();
	if (DamageCauser)
	{
		AMultiFPSPlayerController* MultiFPSPlayerController =
			Cast<AMultiFPSPlayerController>(GetController());
		if (MultiFPSPlayerController)
		{
			MultiFPSPlayerController->DeathMatch(DamageCauser);
		}
	}
}

void AHomeworkCharacter::OnHit(AActor* DamagedActor, float Damage, class AController* InstigatedBy,
	FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName,
	FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser)
{
	if (HP == 0)
		return;
	HP = (HP > Damage) ? HP - Damage : 0;
	ClientUpdateHPUI(HP);
	AGrenade* DamageSrc = Cast<AGrenade>(DamageCauser);
	if (HP == 0)
	{
		// 死亡逻辑
		if (DamageSrc)
			DamageCauser = DamageSrc->GrenadeOwner;
		Dead(DamageCauser);
	}
	else
	{
		if (DamageSrc)
			Dead(nullptr);
	}
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HP is :%f"), HP));
}

void AHomeworkCharacter::HitedByAI(AActor* DamageCauser, float Damage)
{
	if (HP == 0)
		return;
	HP = (HP > Damage) ? HP - Damage : 0;
	ClientUpdateHPUI(HP);
	if (HP == 0)
	{
		Dead(DamageCauser);
	}
}

void AHomeworkCharacter::DelayPlayArmReloadCallBack()
{
	AWeaponBaseServer* CurServerWeapon = GetCurrentServerWeapon();
	int32 Dif_Bullet = CurServerWeapon->ClipMaxBullet - CurServerWeapon->ClipCurrentBullet;
	if (CurServerWeapon->GunCurrentBullet > Dif_Bullet)
	{
		CurServerWeapon->GunCurrentBullet -= Dif_Bullet;
		CurServerWeapon->ClipCurrentBullet = CurServerWeapon->ClipMaxBullet;
	}
	else
	{
		CurServerWeapon->ClipCurrentBullet += CurServerWeapon->GunCurrentBullet;
		CurServerWeapon->GunCurrentBullet = 0;
	}
	ClientUpdateBulletUI(CurServerWeapon->ClipCurrentBullet, CurServerWeapon->GunCurrentBullet);
	IsReloading = false;
}

void AHomeworkCharacter::DelayPlayGrenadeExplosionCallBack()
{
	if (CurGrenade)
	{
		CurGrenade->PlayExplosion(this);
	}
	IsExplosion = false;
}

void AHomeworkCharacter::DelayGetControllerCallBack()
{
	FPSPlayerController = Cast<AMultiFPSPlayerController>(GetController());
	if (FPSPlayerController)
	{
		FPSPlayerController->CreatPlayerUI();
	}
	else
	{
		FLatentActionInfo ActionInfo(0, FMath::Rand(), TEXT("DelayGetControllerCallBack"), this);
		UKismetSystemLibrary::Delay(this, 0.5, ActionInfo);
	}
}

void AHomeworkCharacter::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AHomeworkCharacter, IsReloading, COND_None);
	DOREPLIFETIME_CONDITION(AHomeworkCharacter, IsFiring, COND_None);
	DOREPLIFETIME_CONDITION(AHomeworkCharacter, IsAiming, COND_None);
	DOREPLIFETIME_CONDITION(AHomeworkCharacter, IsExplosion, COND_None);
	DOREPLIFETIME_CONDITION(AHomeworkCharacter, ActiveWeapon, COND_None);
}

void AHomeworkCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 初始化
	HP = 100;
	bIsFirstPerson = true;
	IsFiring = false;
	IsReloading = false;
	IsAiming = false;
	IsFirstTouch = true;
	CurGrenade = nullptr;
	TestWeapon = FMath::RandRange(0, 2) > 0 ? EWeaponType::FPS : EWeaponType::Sniper;
	ScreenControl = CreateWidget<UMyUserWidget>(GetWorld(), ScreenControlBPClass);
	ScreenControl->SetCurrPawn(this);
	ScreenControl->AddToViewport();

	OnTakePointDamage.AddDynamic(this, &AHomeworkCharacter::OnHit);

	ClientArmsAnimBP = FPArmsMesh->GetAnimInstance();
	ServerBodysAnimBP = GetMesh()->GetAnimInstance();
	FPSPlayerController = Cast<AMultiFPSPlayerController>(GetController());
	if (FPSPlayerController)
	{
		FPSPlayerController->CreatPlayerUI();
	}
	else
	{
		FLatentActionInfo ActionInfo(0, FMath::Rand(), TEXT("DelayGetControllerCallBack"), this);
		UKismetSystemLibrary::Delay(this, 0.5, ActionInfo);
	}
	StartWithKindofWeapon();
}

void AHomeworkCharacter::FirePressed()
{
	switch (ActiveWeapon)
	{
	case EWeaponType::FPS:
	{
		FireWeaponPrimary();
		break;
	}
	case EWeaponType::Sniper:
	{
		FireWeaponSniper();
		break;
	}
	default:
		break;
	}
}

void AHomeworkCharacter::FireReleased()
{
	switch (ActiveWeapon)
	{
	case EWeaponType::FPS:
	{
		StopFireWeaponPrimary();
		break;
	}
	default:
		break;
	}
}

void AHomeworkCharacter::AimPressed()
{
	if (ActiveWeapon == EWeaponType::Sniper)
	{
		ServerSetAiming();
		ClientAiming();
	}
}

void AHomeworkCharacter::Reload()
{
	if (!IsReloading && !IsFiring)
	{
		switch (ActiveWeapon)
		{
		case EWeaponType::FPS:
		case EWeaponType::Sniper:
		{
			ReloadWeaponPrimary();
			break;
		}
		default:
			break;
		}
	}
}

void AHomeworkCharacter::PitchGrenade()
{
	if (!IsFiring && !IsReloading && !IsExplosion)
	{
		ServerGrenadeExplode();
		//  客户端位置不同，是摄像机位置问题 ,在服务器上调用，已解决
	}
}

void AHomeworkCharacter::LockDirection()
{
	IsLockDirection = true;
	CurYawRotation = Controller->GetControlRotation();
}

void AHomeworkCharacter::UnLockDirection()
{
	IsLockDirection = false;
}

void AHomeworkCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AHomeworkCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AHomeworkCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		FRotator Rotation = Controller->GetControlRotation();
		if (IsLockDirection)
			Rotation = CurYawRotation;
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AHomeworkCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		FRotator Rotation = Controller->GetControlRotation();
		if (IsLockDirection)
			Rotation = CurYawRotation;
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
