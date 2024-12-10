// Fill out your copyright notice in the Description page of Project Settings.

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
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnable(ECollisionEnabled::Type CollisionEnabled);

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

protected:
	virtual void BeginPlay() override;
	virtual void Attack();
	virtual bool CanAttack();
	virtual bool CanDodge();
	UFUNCTION(BlueprintNativeEvent)
	void Die();
	virtual bool IsAlive();
	void PlayDodgeMontage();



	virtual void PlayHitReactMontage(const FName& SectionName);
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void HandleDamage(float Damage);
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	void PlayMontage(UAnimMontage* Montage);
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);
	virtual int32 PlayDeathMontage();
	virtual int32 PlayAttackMontage();
	virtual void StopAttackMontage();
	void DisableCapsule();
	void DirectionalHitReact(const FVector& ImpactPoint);

	
	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();
	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();
	UPROPERTY(EditAnywhere, category = "Category")
	double WarpTargetOffset = 75.f;


	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();
	UFUNCTION(BlueprintCallable)
	virtual void DodgeEnd();

	UPROPERTY(EditDefaultsOnly, category = "Montages")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, category = "Montages")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, category = "Montages")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, category = "Montages")
	UAnimMontage* DodgeMontage;

	UPROPERTY(EditDefaultsOnly, category = "Audio")
	USoundBase* HitReactSound;

	UPROPERTY(EditAnywhere, category = "Combat")
	TArray<FName> AttackMontageSections;

	UPROPERTY(EditAnywhere, category = "Combat")
	TArray<FName> DeathMontageSections;

	UPROPERTY(EditAnywhere, category = "FX")
	UParticleSystem* HitParticles;

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	UPROPERTY(VisibleInstanceOnly)
	AWeapon* EquippedItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AActor* CombatTarget;
};
