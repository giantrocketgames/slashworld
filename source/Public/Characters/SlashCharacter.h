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
	/** AActor */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
	/** /AActor */

	/** IHitInterface */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/** /IHitInterface */

	/** IPickupInterface */
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void PickupTreasure(class ATreasure* Treasure) override;
	virtual void PickupSoul(class ASoul* Soul) override;
	/** /IPickupInterface */

protected:
	virtual void BeginPlay() override;
	virtual bool CanAttack() override;
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual void Die_Implementation() override;
	virtual bool CanDodge() override;
	virtual void HandleDamage(float Damage) override;

	UFUNCTION(BlueprintCallable)
	void EquipEnd(const bool bEquipped);
	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

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

	UPROPERTY(VisibleInstanceOnly, category = "State")
	AItem* OverlappingItem;

	UPROPERTY(VisibleInstanceOnly, category = "State")
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, category = "State")
	EActionState ActionState = EActionState::EAS_Unoccupied;

private:
	void InitializeHUD();
	bool CanDisarm();
	bool CanArm();
	bool CanMove();
	bool CanJump();
	void PlayEquipMontage(const bool bEquip);
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void DoJump(const FInputActionValue& Value);
	void Pickup(const FInputActionValue& Value);
	void DoAttack(const FInputActionValue& Value);
	void DoDodge(const FInputActionValue& Value);

	UPROPERTY()
	class USlashOverlay* SlashOverlay;

public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
};
