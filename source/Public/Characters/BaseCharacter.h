#pragma once

#include "CoreMinimal.h"
#include "Interfaces/HitInterface.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnable(ECollisionEnabled::Type CollisionEnabled);

	/** IHitInterface */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/** /IHitInterface */

protected:
	/** AActor */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	/** /AActor */

	/** Combat Functions **/
	virtual void HandleDamage(float Damage);
	virtual bool CanAttack();
	virtual bool CanDodge();
	virtual void Attack();
	virtual bool IsAlive();

	/** Montages **/
	virtual void PlayHitReactMontage(const FName& SectionName);
	virtual int32 PlayDeathMontage();
	virtual int32 PlayAttackMontage();
	virtual void StopAttackMontage();
	void PlayDodgeMontage();
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	void PlayMontage(UAnimMontage* Montage);
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);

	/** Blueprint Native/Callable Functions **/
	UFUNCTION(BlueprintNativeEvent)
	void Die();
	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();	// Called by the animation blueprint when the montage notifies the attack is almost over
	UFUNCTION(BlueprintCallable)
	virtual void DodgeEnd();	// Called by the animation blueprint when the montage notifies the dodge is almost over
	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();
	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();


	/** Properties **/
	UPROPERTY(EditDefaultsOnly, category = "Montages")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, category = "Montages")
	UAnimMontage* HitReactMontage;
	
	UPROPERTY(EditDefaultsOnly, category = "Montages")
	bool bUseDirectionalHitReact = false;

	UPROPERTY(EditDefaultsOnly, category = "Montages")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, category = "Montages")
	UAnimMontage* DodgeMontage;

	UPROPERTY(EditAnywhere, category = "Montages")
	TArray<FName> AttackMontageSections;

	UPROPERTY(EditAnywhere, category = "Montages")
	TArray<FName> DeathMontageSections;

	UPROPERTY(EditDefaultsOnly, category = "Audio")
	USoundBase* HitReactSound;

	UPROPERTY(EditAnywhere, category = "Combat")
	UParticleSystem* HitParticles;

	UPROPERTY(VisibleAnywhere, category = "Combat")
	UAttributeComponent* Attributes;

	UPROPERTY(VisibleInstanceOnly, category = "Combat")
	AWeapon* EquippedItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = "Combat")
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere, category = "Combat")
	double WarpTargetOffset = 75.f;

private:
	void DirectionalHitReact(const FVector& ImpactPoint);

	void DisableCapsule();
};
