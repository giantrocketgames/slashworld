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

	InitializeHUD();
}

/// <summary>
/// Initializes the HUD and caches SlashOverlay
/// </summary>
void ASlashCharacter::InitializeHUD()
{
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

/// <summary>
/// Bound to input to move the character if they are able to
/// </summary>
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

/// <summary>
/// Bound to input to rotate the camera & look around
/// </summary>
void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D move = Value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(move.X);
		AddControllerPitchInput(move.Y);
	}
}

/// <summary>
/// Bound to input to make the character jump if it's able to
/// </summary>
/// <param name="Value"></param>
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

/// <summary>
/// Bound to input to pickup the overlapping item OR arm/disarm our current weapon
/// </summary>
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

/// <summary>
/// Bound to input to trigger an attack if able
/// </summary>
void ASlashCharacter::DoAttack(const FInputActionValue& Value)
{
	if (!CanAttack())
	{
		return;
	}

	ActionState = EActionState::EAS_Attacking;
	Attack();
}

/// <summary>
/// Bound to input to make the player dodge if able
/// </summary>
void ASlashCharacter::DoDodge(const FInputActionValue& Value)
{
	if (!CanMove() || !CanDodge())
	{
		return;
	}
	ActionState = EActionState::EAS_Dodge;
	PlayDodgeMontage();
}
/// <summary>
/// Plays the Equip montage to arm/disarm our current weapon
/// </summary>
/// <param name="bEquip">True moves from back->hand. False does hand->back</param>
void ASlashCharacter::PlayEquipMontage(const bool bEquip)
{
	if (EquipMontage)
	{
		FName SectionName = bEquip ? FName("Equip") : FName("Unequip");
		PlayMontageSection(EquipMontage, SectionName);
	}
}

/// <summary>
/// Called via Anim Notify on the animation blueprint when the Attack Montage is finished
/// </summary>
void ASlashCharacter::AttackEnd()
{
	Super::AttackEnd();
	ActionState = EActionState::EAS_Unoccupied;
}

/// <summary>
/// Called via Anim Notify on the animation blueprint when the Dodge Montage is finished
/// </summary>
void ASlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();
	ActionState = EActionState::EAS_Unoccupied;
}

/// <summary>
/// Called via Anim Notify on the animation blueprint when the Hit React Montage is finished
/// </summary>
void ASlashCharacter::HitReactEnd()
{
	if (IsAlive())
	{
		ActionState = EActionState::EAS_Unoccupied;
	}
}

/// <summary>
/// Called via Anim Notify on the animation blueprint when the Equip Montage is finished
/// </summary>
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

/// <summary>
/// Overridden implementation of Die.
/// Only custom behavior is setting of states
/// </summary>
void ASlashCharacter::Die_Implementation()
{
	Super::Die_Implementation();
	CharacterState = ECharacterState::ECS_Dead;
	ActionState = EActionState::EAS_Dead;
}

/// <summary>
/// Called when the character takes damage
/// Updates SlashOverlay after we take damage
/// </summary>
void ASlashCharacter::HandleDamage(float Damage)
{
	Super::HandleDamage(Damage);
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

/// <summary>
/// Implementation for GetHit
/// Sets ActionState and then does default behavior
/// </summary>
void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	ActionState = EActionState::EAS_HitReaction;
	Super::GetHit_Implementation(ImpactPoint, Hitter);
}

/// <summary>
/// Helper to control State
/// </summary>
bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped;
}

/// <summary>
/// Helper to control State
/// </summary>
bool ASlashCharacter::CanDisarm()
{
	return CharacterState != ECharacterState::ECS_Unequipped && ActionState == EActionState::EAS_Unoccupied;
}

/// <summary>
/// Helper to control State
/// </summary>
bool ASlashCharacter::CanArm()
{
	return CharacterState == ECharacterState::ECS_Unequipped && ActionState == EActionState::EAS_Unoccupied;;
}

/// <summary>
/// Helper to control State
/// </summary>
bool ASlashCharacter::CanMove()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

/// <summary>
/// Helper to control State
/// </summary>
bool ASlashCharacter::CanJump()
{
	return CanMove();
}

/// <summary>
/// Helper to control State
/// </summary>
bool ASlashCharacter::CanDodge()
{
	return ActionState == EActionState::EAS_Unoccupied && Super::CanDodge();
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

/// <summary>
/// Set when the character is overlapping with an item in the world
/// </summary>
void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}


/// <summary>
/// Set when the character is overlapping with treasure in the world
/// </summary>
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

/// <summary>
/// Set when the character is overlapping with souls in the world
/// </summary>
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

