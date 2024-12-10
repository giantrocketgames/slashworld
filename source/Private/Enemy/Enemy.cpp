// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Animation/AnimInstance.h"
#include "AIController.h"
#include "Items/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "HUD/HealthBarComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationPath.h"
#include "Components/AttributeComponent.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());


	PawnSensor = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensor"));
	PawnSensor->SetPeripheralVisionAngle(45.f);
	PawnSensor->SightRadius = 4000.f;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(1.f);
		ToggleHealthBar(false);
	}

	AIController = Cast<AAIController>(GetController());
	
	if (PawnSensor)
	{
		PawnSensor->OnSeePawn.AddDynamic(this, &AEnemy::OnPawnSeen);
	}

	SpawnDefaultWeapon();

	GetPatrolTarget();
	if (PatrolTarget)
	{
		EnemyState = EEnemyState::EES_Patrolling;
		MoveToTarget(PatrolTarget);
	}

	Tags.Add(FName("Enemy"));
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	SetCombatTarget(EventInstigator->GetPawn());
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AEnemy::Destroyed()
{
	if (EquippedItem)
	{
		EquippedItem->Destroy();
	}
}

void AEnemy::SetCombatTarget(APawn* Target)
{
	if (Target == nullptr)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Enemy::SetCombatTarget"));
	CombatTarget = Target;
	if (!IsOutsideAttackRadius())
	{
		EnemyState = EEnemyState::EES_Attacking;
	}
	else if (IsOutsideAttackRadius())
	{
		ClearPatrolTimer();
		StartChasing();
	}
}

void AEnemy::OnPawnSeen(APawn* Pawn)
{
	if (Pawn == nullptr || EnemyState >= EEnemyState::EES_Chasing || EnemyState == EEnemyState::EES_Dead) return;
	
	if (Pawn->ActorHasTag(FName("EngageableTarget")) && !Pawn->ActorHasTag(FName("Dead")))
	{
		SetCombatTarget(Pawn);
	}
}

void AEnemy::Die_Implementation()
{
	Super::Die_Implementation();
	ClearAttackTimer();
	GetCharacterMovement()->bOrientRotationToMovement = false;
	ToggleHealthBar(false);

	SetLifeSpan(DeathLifeSpan);
	EnemyState = EEnemyState::EES_Dead;

	if (ItemToDrop)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 25.f);
			World->SpawnActor<AItem>(ItemToDrop, SpawnLocation, GetActorRotation());
		}
	}
}

bool AEnemy::CanAttack()
{
	return EnemyState < EEnemyState::EES_Attacking && !IsOutsideAttackRadius() && EnemyState != EEnemyState::EES_Dead;
}

bool AEnemy::InTargetRange(AActor* Actor, double Radius)
{
	if (Actor == nullptr) return false;

	const double DistanceToTarget = (Actor->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(MoveToAcceptanceRadius);
	AIController->MoveTo(MoveRequest);
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("WeaponSocket"), this, this);
		EquippedItem = DefaultWeapon;
	}
}

void AEnemy::ToggleHealthBar(bool bShow)
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(bShow);
	}
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	ToggleHealthBar(false);
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrolWalkSpeed;
	MoveToTarget(PatrolTarget);
	UE_LOG(LogTemp, Warning, TEXT("Enemy::CheckCombatTarget::Start Patrol"));
}

void AEnemy::StartChasing()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChaseWalkSpeed;
	MoveToTarget(CombatTarget);
	UE_LOG(LogTemp, Warning, TEXT("Enemy::CheckCombatTarget::Chasing"));
}

bool AEnemy::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

void AEnemy::StartAttackTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy::CheckCombatTarget::Starting Attack"));
	EnemyState = EEnemyState::EES_Attacking;
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackDelay);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemy::HandleDamage(float Damage)
{
	Super::HandleDamage(Damage);

	if (HealthBarWidget && IsAlive())
	{
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
		ToggleHealthBar(true);
	}
}

void AEnemy::Attack()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy::Attack"));
	if (CombatTarget && CombatTarget->ActorHasTag("Dead"))
	{
		CombatTarget = nullptr;
		return;
	}
	EnemyState = EEnemyState::EES_Engaged;
	Super::Attack();
}

void AEnemy::AttackEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy::AttackEnd"));
	Super::AttackEnd();
	EnemyState = EEnemyState::EES_Idle;
	
	CheckCombatTarget();
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{

	Super::GetHit_Implementation(ImpactPoint, Hitter);

	ClearPatrolTimer();
	ClearAttackTimer();

	StopAttackMontage();

	if (!IsOutsideAttackRadius())
	{
		if (IsAlive()) // Note: We get Hit then take damage so this may start attack timer even though we will be dead shortly
		{
			StartAttackTimer();
		}
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (EnemyState == EEnemyState::EES_Dead) return;

	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
	}
	else
	{
		CheckPatrolTarget();
	}

}

void AEnemy::CheckPatrolTarget()
{
	AActor* OldPatrolTarget = PatrolTarget;
	EnemyState = EEnemyState::EES_Patrolling;
	if (InTargetRange(PatrolTarget, 200.f))
	{
		GetPatrolTarget();

		if (PatrolTarget == nullptr || PatrolTarget == OldPatrolTarget)
		{
			return;
		}

		const float PauseDelay = FMath::RandRange(0.5f, 5.f);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, PauseDelay);
	}
}

void AEnemy::GetPatrolTarget()
{
	if (!PatrolMarkers.IsEmpty())
	{
		const int32 PatrolIdx = FMath::RandRange(0, PatrolMarkers.Num() - 1);
		PatrolTarget = PatrolMarkers[PatrolIdx];
	}
	else
	{
		PatrolTarget = nullptr;
	}
}

void AEnemy::CheckCombatTarget()
{
	if (IsOutsideCombatRadius())
	{
		ClearAttackTimer();
		LoseInterest();
		if (EnemyState != EEnemyState::EES_Engaged)
		{
			StartPatrolling();
		}
	}
	else if (EnemyState != EEnemyState::EES_Chasing && IsOutsideAttackRadius())
	{
		ClearAttackTimer();
		if (EnemyState != EEnemyState::EES_Engaged)
		{
			StartChasing();
		}
	}
	else if (CanAttack())
	{
		StartAttackTimer();
	}
}

