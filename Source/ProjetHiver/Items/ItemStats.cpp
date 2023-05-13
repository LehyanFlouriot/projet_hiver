#include "ItemStats.h"

FItemStats::FItemStats() : FItemStats(0, 0, 0, 0, 0, 0, 0, 0, 0) {}

FItemStats::FItemStats(const int PhysicalPower, const int PhysicalDefense, const int BlockingPhysicalDefense,
					   const int MagicalPower, const int MagicalDefense, const int BlockingMagicalDefense,
					   const int MaxHealth, const int MaxPotionCharge, const int MovementSpeed)
	: PhysicalPower(PhysicalPower), PhysicalDefense(PhysicalDefense), BlockingPhysicalDefense(BlockingPhysicalDefense),
	  MagicalPower(MagicalPower), MagicalDefense(MagicalDefense), BlockingMagicalDefense(BlockingMagicalDefense),
	  MaxHealth(MaxHealth), MaxPotionCharge(MaxPotionCharge), MovementSpeed(MovementSpeed) {}
