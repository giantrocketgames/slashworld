// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "CharacterTypes.h"
#include "Interfaces/PickupInterface.h"
#include "SlashCharacter.generated.h"


class UInputMappingContext;
class UInputAction;
class AItem;
class UAnimMontage;
class ASoul;

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	ASlashCharacter();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
	/** IHitInterface */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/** /IHitInterface */

	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void PickupTreasure(class ATreasure* Treasure) override;
	virtual void PickupSoul(class ASoul* Soul) override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Input")
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Input")
	UInputAction* LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Input")
	UInputAction* PickupAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Input")
	UInputAction* DodgeAction;

	UPROPERTY(EditDefaultsOnly, category = "Montages")
	UAnimMontage* EquipMontage;

	virtual void BeginPlay() override;
	virtual bool CanAttack() override;
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual void Die_Implementation() override;
	virtual bool CanDodge() override;
	virtual void HandleDamage(float Damage) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void DoJump(const FInputActionValue& Value);
	void Pickup(const FInputActionValue& Value);
	void DoAttack(const FInputActionValue& Value);
	void DoDodge(const FInputActionValue& Value);
	
	void PlayEquipMontage(const bool bEquip);
	
	UFUNCTION(BlueprintCallable)
	void EquipEnd(const bool bEquipped);
	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

	bool CanDisarm();
	bool CanArm();
	bool CanMove();
	bool CanJump();

private:
	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(VisibleInstanceOnly)
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY()
	class USlashOverlay* SlashOverlay;

public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
};
