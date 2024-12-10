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

/// <summary>
/// Plays the attack montage
/// Default implementation picks a random section defined by AttackMontageSections
/// </summary>
/// <returns>Index of the section played</returns>
int32 ABaseCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}

/// <summary>
/// Stops the Attack Montage if it's playing
/// </summary>
void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.25f, AttackMontage);
	}
}

/// <summary>
/// Helper to disable the collision on the Capsule Component
/// </summary>
void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

/// <summary>
/// Plays a directional hit react based on ImpactPoint 
/// if bUseDirectionalHitReact is false, it will play the default section
/// </summary>
/// <param name="ImpactPoint">Location the hit was from</param>
void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	if (!bUseDirectionalHitReact)
	{
		PlayHitReactMontage(FName("Default"));
		return;
	}
	
	const FVector Forward = GetActorForwardVector();
	const FVector ImpactLower = FVector(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLower - GetActorLocation()).GetSafeNormal();

	// Dot Product will tell us the angle but does not tell us the direction
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	double Theta = FMath::Acos(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);

	const FVector CrossProd = FVector::CrossProduct(Forward, ToHit);

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

	PlayHitReactMontage(Section);
}

/// <summary>
/// Gets the Motion Warping location target based on CombatTarget's location
/// This is useful to make sure enemies move into range to hit the player 
/// </summary>
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

/// <summary>
/// Gets the Motion Warping rotation target based on CombatTarget
/// This is useful to help enemies always face the player before attacking
/// </summary>
FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (CombatTarget)
	{
		return CombatTarget->GetActorLocation();
	}
	return FVector();
}

/// <summary>
/// Tells the character to attack
/// Base behavior plays the attack montage
/// </summary>
void ABaseCharacter::Attack()
{
	PlayAttackMontage();
}

/// <summary>
/// Getter to determine if the character can attack or not
/// Note: Base behavior is always false!
/// </summary>
bool ABaseCharacter::CanAttack()
{
	return false;
}

/// <summary>
/// Getter to determine if the character can dodge or not
/// </summary>
bool ABaseCharacter::CanDodge()
{
	if (Attributes)
	{
		return Attributes->CanUseDodge();
	}
	return false;
}
/// <summary>
/// Base implementation for when the character dies
/// Adds the Dead tag, plays the death montage, and disables collision
/// </summary>
void ABaseCharacter::Die_Implementation()
{
	Tags.Add(FName("Dead"));
	if (DeathMontage)
	{
		PlayMontage(DeathMontage);
	}
	DisableCapsule();
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetWeaponCollisionEnable(ECollisionEnabled::NoCollision);
}

/// <summary>
/// Helper for determining if the character is still alive
/// </summary>
/// <returns></returns>
bool ABaseCharacter::IsAlive()
{
	return Attributes != nullptr && Attributes->IsAlive();
}

/// <summary>
/// Implementation of GetHit
/// Disables weapon collision
/// Starts to play a hit reaction
/// Plays the hit react sound
/// Plays hit react particles
/// </summary>
/// <param name="ImpactPoint">Point the actor was hit at</param>
/// <param name="Hitter">The hitter who triggered this event</param>
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

/// <summary>
/// The event class for an Actor taking damage
/// </summary>
float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	return DamageAmount;
}

/// <summary>
/// Handles taking damage and will potentially trigger Die on the character
/// </summary>
/// <param name="Damage">Damage to apply</param>
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

/// <summary>
/// Plays the specified montage section from the corresponding montage.
/// </summary>
/// <param name="Montage">Montage to play</param>
/// <param name="SectionName">Section to jump to</param>
void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

/// <summary>
/// Plays the provided Montage
/// </summary>
/// <param name="Montage">Montage to play</param>
void ABaseCharacter::PlayMontage(UAnimMontage* Montage)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(Montage);
	}
}

/// <summary>
/// Plays a random montage section from the list of provided SectionNames
/// If the SectionNames is empty it will simply play the Montage
/// </summary>
/// <param name="Montage">Montage to play</param>
/// <param name="SectionNames">Name of sections to pick from</param>
/// <returns>Index randomly chosen</returns>
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

/// <summary>
/// Plays the death montage.
/// Default behavior picks a random section based on DeathMontageSections
/// </summary>
/// <returns>Index of the section played</returns>
int32 ABaseCharacter::PlayDeathMontage()
{
	return PlayRandomMontageSection(DeathMontage, DeathMontageSections);
}

/// <summary>
/// Plays the dodge montage and deducts the stamina cost from Attributes
/// </summary>
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

/// <summary>
/// Plays the specified section from the hit react montage
/// </summary>
/// <param name="SectionName">Section name to play</param>
void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	PlayMontage(HitReactMontage);
}

/// <summary>
/// Invoked via blueprint's anim notify event when the dodge montage is finished
/// </summary>
void ABaseCharacter::DodgeEnd()
{
}

/// <summary>
/// Invoked when the AnimNotify event is triggered on the blueprint to indicate the attack montage is done
/// </summary>
void ABaseCharacter::AttackEnd()
{
}

/// <summary>
/// Sets the collision enabled flag on the currently Equipped Weapon
/// Typically invoked in blueprints via the anim notify event from the attack montage
/// </summary>
/// <param name="CollisionEnabled"></param>
void ABaseCharacter::SetWeaponCollisionEnable(ECollisionEnabled::Type CollisionEnabled)
{
	if (!EquippedItem)
	{
		return;
	}

	EquippedItem->SetWeaponCollisionEnable(CollisionEnabled);
}

