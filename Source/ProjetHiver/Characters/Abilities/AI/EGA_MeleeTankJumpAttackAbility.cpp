#include "ProjetHiver/Characters/Abilities/AI/EGA_MeleeTankJumpAttackAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/BaseNPC.h"

void UEGA_MeleeTankJumpAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                      const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
		{
			if (IsValid(MontageTask)) MontageTask->EndTask();
			if (IsValid(MeleeStrikeEventTask)) MeleeStrikeEventTask->EndTask();
			if (IsValid(ProgressMultiHitEventTask)) ProgressMultiHitEventTask->EndTask();
			});

	if (!CommitCheck(Handle, ActorInfo, ActivationInfo)) EndAbility(Handle, ActorInfo, ActivationInfo, true, true);

	AbilityProgress = 0;

	ABaseCharacter* Character = CastChecked<ABaseCharacter>(CurrentActorInfo->OwnerActor);
	Character->SetMeleeSphereRadius(SphereInfos[AbilityProgress].SphereRadius);
	Character->SetMeleeSphereOffset(SphereInfos[AbilityProgress].SphereOffset);

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_MeleeTankJumpAttackAbility::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_MeleeTankJumpAttackAbility::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_MeleeTankJumpAttackAbility::OnMontageCompleted);

	MeleeStrikeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.MeleeStrike")));
	MeleeStrikeEventTask->EventReceived.AddDynamic(this, &UEGA_MeleeTankJumpAttackAbility::OnMeleeStrikeEventReceived);

	ProgressMultiHitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.ProgressMultiHit")));
	ProgressMultiHitEventTask->EventReceived.AddDynamic(this, &UEGA_MeleeTankJumpAttackAbility::OnProgressMultiHitEventReceived);

	MontageTask->ReadyForActivation();
	MeleeStrikeEventTask->ReadyForActivation();
	ProgressMultiHitEventTask->ReadyForActivation();

	const ABaseNPC* NPC = Cast<ABaseNPC>(Character);
	const ABaseCharacter* Target = NPC->GetTarget();

	const FVector TargetVelocity = Target->GetVelocity();
	FVector AbilityDirection = (Target->GetActorLocation() + TargetVelocity) - NPC->GetActorLocation();
	AbilityDirection.Normalize();

	DrawDebugSphere(GetWorld(), (Target->GetActorLocation() + TargetVelocity), 50, 30, FColor::Black, false, 5);

	//Random offset ici
	Character->GetCharacterMovement()->AddImpulse((AbilityDirection + FVector::UpVector) * ImpulseForce, true);
}
