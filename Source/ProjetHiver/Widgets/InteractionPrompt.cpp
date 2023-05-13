#include "InteractionPrompt.h"

#include "Components/TextBlock.h"

void UInteractionPrompt::SetText(const FText& Text) const
{
	InteractionText->SetText(Text);
}
