#pragma once


UENUM(BlueprintType)
enum class ECharacterState : uint8// class keyword makes it scoped so usage needs to be fully qualified
{
	ECS_Unequipped UMETA(DisplayName = "Unequipped"),
	ECS_OneHandWeapon UMETA(DisplayName = "OneHandWeapon"),
	ECS_TwoHandWeapon UMETA(DisplayName = "TwoHandWeapon"),
	ECS_Dead
};



UENUM(BlueprintType)
enum class EActionState : uint8
{
	EAS_Unoccupied,
	EAS_HitReaction,
	EAS_Attacking,
	EAS_Equipping,
	EAS_Dodge,
	EAS_Dead
};


UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	EES_Dead,
	EES_Idle,
	EES_Patrolling,
	EES_Chasing,
	EES_Attacking,
	EES_Engaged
};