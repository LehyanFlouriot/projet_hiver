#include "EGA_MagicBeam.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"

UEGA_MagicBeam::UEGA_MagicBeam()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action")));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsAiming")));

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));
}

void UEGA_MagicBeam::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
	{
		if (IsValid(StartMontageTask)) StartMontageTask->EndTask();
		if (IsValid(LoopMontageTask)) LoopMontageTask->EndTask();
		if (IsValid(WaitDelayTask)) WaitDelayTask->EndTask();
		if (IsValid(WaitInputReleaseTask)) WaitInputReleaseTask->EndTask();

		MontageStop();
		K2_RemoveGameplayCue(BeamCueTag);
		BP_RemoveGameplayEffectFromOwnerWithHandle(SlowEffectHandle);

		if (const APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwningActorFromActorInfo()); IsValid(Player)) {
			Player->SetLeftWeaponVisibility(false);
		}
	});

	if (!CommitCheck(Handle, ActorInfo, ActivationInfo)) CancelAbility(Handle, ActorInfo, ActivationInfo, true);

	InitCooldownOnWidget();

	StartMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, StartAimingMontage, StartAnimationPlayingRate);
	StartMontageTask->OnBlendOut.AddDynamic(this, &UEGA_MagicBeam::OnStartMontageBlendOut);
	StartMontageTask->ReadyForActivation();

	WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	WaitInputReleaseTask->OnRelease.AddDynamic(this, &UEGA_MagicBeam::OnInputRelease);
	WaitInputReleaseTask->ReadyForActivation();

	SlowEffectHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,
		MakeOutgoingGameplayEffectSpec(SlowEffect));

	if (const APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwningActorFromActorInfo()); IsValid(Player)) {
		Player->SetLeftWeaponVisibility(true);
	}
}

void UEGA_MagicBeam::OnStartMontageBlendOut()
{
	LoopMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AimingLoopMontage, LoopingAnimationPlayingRate);
	LoopMontageTask->ReadyForActivation();

	CreateTickTask();

	if (APlayerCharacter* Source = Cast<APlayerCharacter>(GetOwningActorFromActorInfo()); IsValid(Source))
	{
		FGameplayCueParameters GameplayCueParameters;
		GameplayCueParameters.Instigator = Source;
		K2_AddGameplayCueWithParams(BeamCueTag, GameplayCueParameters);
	}
}

void UEGA_MagicBeam::OnDelayFinished()
{
	if (APlayerCharacter* Source = Cast<APlayerCharacter>(GetOwningActorFromActorInfo()); IsValid(Source))
	{
		TArray<FHitResult> OutHits;
		const FVector Start = Source->GetMesh()->GetSocketLocation(SocketToFireFrom);
		const FVector End = Source->GetActorLocation() + Source->GetActorRotation().RotateVector(Source->GetCameraBoom()->SocketOffset) + Source->GetControlRotation().Vector() * MaxRange;
		FCollisionQueryParams Params{ {}, false, GetOwningActorFromActorInfo() };
		
		GetWorld()->LineTraceMultiByChannel(OutHits, Start, End, ECC_Pawn, Params);

		if (Source->GetDebugAbilitiesHitbox())
			DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1, 0, 5);

		TArray<AActor*> HitActors;

		for (auto OutHit : OutHits)
			if (ABaseCharacter* Target = Cast<ABaseCharacter>(OutHit.GetActor()); IsValid(Target))
				if (Source->GetFactionTag() != Target->GetFactionTag() && Target->IsAliveHealth() &&
					!Target->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsInvincible"))) && !HitActors.Contains(Target))
				{
						ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,
					CreateDamageEffect(DamageEffect, DamageInfo), UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Target));
						HitActors.Add(Target);
				}

		CreateTickTask();
	}
}

void UEGA_MagicBeam::OnInputRelease(float DeltaTime)
{
	CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_MagicBeam::CreateTickTask()
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, TickLength);
		WaitDelayTask->OnFinish.AddDynamic(this, &UEGA_MagicBeam::OnDelayFinished);
		WaitDelayTask->ReadyForActivation();
	}
}
