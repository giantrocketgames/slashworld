// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/StaticMeshComponent.h"
#include "Item.h"
#include "Items/Weapon.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Components/BoxComponent.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Components/AttributeComponent.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* playerController = Cast<APlayerController>(Controller);
	if (playerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	Tags.Add(FName("EngageableTarget"));

	AController* ActorController = GetController();
	if (ActorController)
	{
		APlayerController* PlayerController = Cast<APlayerController>(ActorController);
		if (PlayerController)
		{
			ASlashHUD* SlashHud = Cast<ASlashHUD>(PlayerController->GetHUD());
			if (SlashHud)
			{
				SlashOverlay = SlashHud->GetSlashOverlay();
				if (SlashOverlay)
				{
					SlashOverlay->SetHealthBarPercent(1.f);
					SlashOverlay->SetStaminaBarPercent(1.f);
					SlashOverlay->SetCoinsValue(0);
					SlashOverlay->SetSoulsValue(0);
				}
			}
		}
	}
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (!CanMove())
	{
		return;
	}

	const FVector2D move = Value.Get<FVector2D>();
	AController* controller = GetController();
	if (controller)
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, move.Y);
		AddMovementInput(Right, move.X);
	}
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D move = Value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(move.X);
		AddControllerPitchInput(move.Y);
	}
}

void ASlashCharacter::DoJump(const FInputActionValue& Value)
{
	if (!CanJump())
	{
		return;
	}

	if (GetController())
	{
		ACharacter::Jump();
	}
}

void ASlashCharacter::Pickup(const FInputActionValue& Value)
{
	if (OverlappingItem)
	{
		AWeapon* Weapon = Cast<AWeapon>(OverlappingItem);
		if (Weapon)
		{
			Weapon->Equip(GetMesh(), FName("SpineSocket"), this, this);

			EquippedItem = Weapon;
			SetOverlappingItem(nullptr);
			CharacterState = ECharacterState::ECS_Unequipped;
		}
	}
	else 
	{
		if (CanDisarm())
		{
			PlayEquipMontage(false);
			ActionState = EActionState::EAS_Equipping;
		}
		else if (CanArm() && EquippedItem)
		{
			PlayEquipMontage(true);
			ActionState = EActionState::EAS_Equipping;
		}
	}
}

void ASlashCharacter::DoAttack(const FInputActionValue& Value)
{
	if (!CanAttack())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("DoAttack!"));
	ActionState = EActionState::EAS_Attacking;
	Attack();
}

void ASlashCharacter::DoDodge(const FInputActionValue& Value)
{
	if (!CanMove() || !CanDodge()) { return; }
	ActionState = EActionState::EAS_Dodge;
	PlayDodgeMontage();
}

void ASlashCharacter::PlayEquipMontage(const bool bEquip)
{
	if (EquipMontage)
	{
		FName SectionName = bEquip ? FName("Equip") : FName("Unequip");
		PlayMontageSection(EquipMontage, SectionName);
	}
}

void ASlashCharacter::AttackEnd()
{
	Super::AttackEnd();
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::Die_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("PLAYER DEAD"));
	Super::Die_Implementation();
	CharacterState = ECharacterState::ECS_Dead;
	ActionState = EActionState::EAS_Dead;
}

bool ASlashCharacter::CanDodge()
{
	return ActionState == EActionState::EAS_Unoccupied && Super::CanDodge();
}

void ASlashCharacter::HandleDamage(float Damage)
{
	Super::HandleDamage(Damage);
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void ASlashCharacter::EquipEnd(const bool bEquipped)
{
	ActionState = EActionState::EAS_Unoccupied;
	if (bEquipped)
	{
		EquippedItem->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		CharacterState = ECharacterState::ECS_TwoHandWeapon;
	}
	else
	{
		EquippedItem->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
		CharacterState = ECharacterState::ECS_Unequipped;
	}
}

void ASlashCharacter::HitReactEnd()
{
	if (IsAlive())
	{
		ActionState = EActionState::EAS_Unoccupied;
	}
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanDisarm()
{
	return CharacterState != ECharacterState::ECS_Unequipped && ActionState == EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanArm()
{
	return CharacterState == ECharacterState::ECS_Unequipped && ActionState == EActionState::EAS_Unoccupied;;
}

bool ASlashCharacter::CanMove()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanJump()
{
	return CanMove();
}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::DoJump);
		EnhancedInputComponent->BindAction(PickupAction, ETriggerEvent::Started, this, &ASlashCharacter::Pickup);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::DoAttack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASlashCharacter::DoDodge);
	}
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	ActionState = EActionState::EAS_HitReaction;
	Super::GetHit_Implementation(ImpactPoint, Hitter);
}

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void ASlashCharacter::PickupTreasure(ATreasure* Treasure)
{
	if (Attributes)
	{
		Attributes->ChangeGold(Treasure->GetValue());

		if (SlashOverlay)
		{
			SlashOverlay->SetCoinsValue(Attributes->GetGold());
		}
	}
}

void ASlashCharacter::PickupSoul(ASoul* Soul)
{
	if (Attributes)
	{
		Attributes->ChangeSouls(Soul->GetSoulValue());

		if (SlashOverlay)
		{
			SlashOverlay->SetSoulsValue(Attributes->GetSouls());
		}
	}
}

