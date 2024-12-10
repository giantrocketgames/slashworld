// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UParticleSystem;
class UAttributeComponent;
class UHealthBarComponent;
class UPawnSensingComponent;
class AItem;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	/** <AActor> */
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	/** </AActor> */

	/** IHitInterface */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/** /IHitInterface */

protected:
	/** <AActor> */
	virtual void BeginPlay() override;
	/** </AActor> */

	/** <ABaseCharacter> */
	virtual void Die_Implementation() override;
	virtual void HandleDamage(float Damage) override;
	virtual bool CanAttack() override;
	virtual void Attack() override;
	virtual void AttackEnd() override;
	/** </ABaseCharacter> */


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Idle;

private:

	/** <Navigation> */
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolMarkers;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;

	UPROPERTY(EditAnywhere)
	float WaypointReachedDelay = 1.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float PatrolWalkSpeed = 125.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ChaseWalkSpeed = 300.f;

	UPROPERTY()
	class AAIController* AIController;
	/** </Navigation> */

	
	
	UPROPERTY(EditAnywhere)
	double CombatRadius = 500;

	UPROPERTY(EditAnywhere)
	double AttackRadius = 50;

	UFUNCTION()
	void OnPawnSeen(APawn* Pawn); // Callback for OnPawnSeen in UPawnSensingComponent


	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<class AItem> ItemToDrop;
	
	// AIBehavior
	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();
	void ClearPatrolTimer();
	void LoseInterest();
	void StartPatrolling();
	void StartChasing();
	void CheckPatrolTarget();
	void GetPatrolTarget();
	void CheckCombatTarget();
	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	void SetCombatTarget(APawn* Target);
	float NextDecisionTimer = 0.f;
	void StartAttackTimer();
	void ClearAttackTimer();


	// Navigation
	void MoveToTarget(AActor* Target);
	bool InTargetRange(AActor* Actor, double Radius);


	// Combat
	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackDelay = 0.5f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float DeathLifeSpan = 3.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float MoveToAcceptanceRadius = 15.f;
	
	void SpawnDefaultWeapon();
	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class AWeapon> WeaponClass;

	// UI
	void ToggleHealthBar(bool bShow);


	// Components
	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensor;
};
