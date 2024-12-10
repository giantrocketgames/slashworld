#include "Characters/BaseCharacter.h"
#include "Items/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Components/AttributeComponent.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

int32 ABaseCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}

void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.25f, AttackMontage);
	}
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	const FVector ImpactLower = FVector(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLower - GetActorLocation()).GetSafeNormal();

	// Dot Product will tell us the angle but does not tell us the direction
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	double Theta = FMath::Acos(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);

	const FVector CrossProd = FVector::CrossProduct(Forward, ToHit);
	//Theta *= CrossProd.Z;

	if (CrossProd.Z < 0)
	{
		Theta *= -1.f;
	}

	FName Section("FromBack");

	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("FromFront");
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName("FromLeft");
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName("FromRight");
	}

	//PlayHitReactMontage(Section);

	PlayHitReactMontage(FName("Default"));

}

FVector ABaseCharacter::GetTranslationWarpTarget()
{
	if (CombatTarget == nullptr)
	{
		return FVector();
	}

	const FVector TargetLocation = CombatTarget->GetActorLocation();
	const FVector MyLocation = GetActorLocation();

	FVector TargetToMe = (MyLocation - TargetLocation).GetSafeNormal();
	TargetToMe *= WarpTargetOffset;

	return TargetToMe + TargetLocation;

}

FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (CombatTarget)
	{
		return CombatTarget->GetActorLocation();
	}
	return FVector();
}

void ABaseCharacter::Attack()
{
	PlayAttackMontage();
}

bool ABaseCharacter::CanAttack()
{
	return false;
}

bool ABaseCharacter::CanDodge()
{
	if (Attributes)
	{
		return Attributes->CanUseDodge();
	}
	return false;
}

void ABaseCharacter::Die_Implementation()
{
	Tags.Add(FName("Dead"));
	if (DeathMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("PLAY DEATH MONTAGE"));
		PlayMontage(DeathMontage);
	}
	DisableCapsule();
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetWeaponCollisionEnable(ECollisionEnabled::NoCollision);
}

bool ABaseCharacter::IsAlive()
{
	return Attributes != nullptr && Attributes->IsAlive();
}

void ABaseCharacter::PlayDodgeMontage()
{
	if (Attributes)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
	}

	if (DodgeMontage)
	{
		PlayMontage(DodgeMontage);
	}
}

void ABaseCharacter::DodgeEnd()
{
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	PlayMontage(HitReactMontage);
}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	SetWeaponCollisionEnable(ECollisionEnabled::NoCollision);
	if (IsAlive())
	{
		if (Hitter)
		{
			DirectionalHitReact(Hitter->GetActorLocation());
		}
	}

	if (HitReactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			HitReactSound,
			ImpactPoint
		);
	}

	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitParticles,
			ImpactPoint
		);
	}
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	return DamageAmount;
}

void ABaseCharacter::HandleDamage(float Damage)
{
	if (Attributes)
	{
		Attributes->ChangeHealth(-Damage);

		if (!IsAlive())
		{
			Die();
		}
	}
}

void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

void ABaseCharacter::PlayMontage(UAnimMontage* Montage)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(Montage);
	}
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0)
	{
		PlayMontage(Montage);
		return 0;
	}

	const int32 MaxSelectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSelectionIndex);

	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}

int32 ABaseCharacter::PlayDeathMontage()
{
	return PlayRandomMontageSection(DeathMontage, DeathMontageSections);
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::SetWeaponCollisionEnable(ECollisionEnabled::Type CollisionEnabled)
{
	if (!EquippedItem)
	{
		return;
	}


	EquippedItem->SetWeaponCollisionEnable(CollisionEnabled);
}

