#include "FloatingHealthBar.h"

#include "Components/ProgressBar.h"

void UFloatingHealthBar::SetHealthPercentage(const float NewHealth) const
{
	HealthBar->SetPercent(NewHealth);
}
