#include "EGA_RangeBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Characters/Abilities/Projectiles/RangeBaseProjectile.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"

UEGA_RangeBase::UEGA_RangeBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.RangeAttack"));

	AbilityTags.AddTag(AttackTag);

	ActivationOwnedTags.AddTag(AttackTag);

	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Jump")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Interact")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Potion")));

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));
}

void UEGA_RangeBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(SlowEffectHandle);

		if (IsValid(MontageTask)) MontageTask->EndTask();
		if (IsValid(GameplayEventTask)) GameplayEventTask->EndTask();
	});

	CommitAbility(Handle, ActorInfo, ActivationInfo);

	SlowEffectHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,
		MakeOutgoingGameplayEffectSpec(SlowEffect));

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage, MontagePlayingRate);
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_RangeBase::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_RangeBase::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_RangeBase::OnMontageCompleted);

	GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.SpawnProjectile")));
	GameplayEventTask->EventReceived.AddDynamic(this, &UEGA_RangeBase::OnGameplayEventReceived);

	MontageTask->ReadyForActivation();
	GameplayEventTask->ReadyForActivation();
}

void UEGA_RangeBase::OnMontageCompleted()
{
	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_RangeBase::OnGameplayEventReceived(FGameplayEventData GameplayEventData)
{
	if (HasAuthority(&CurrentActivationInfo))
		if (APlayerCharacter* Source = Cast<APlayerCharacter>(GetOwningActorFromActorInfo()))
		{
			const FVector Start = Source->GetMesh()->GetSocketLocation(SocketToSpawnProjectile);
			const FVector End = Source->GetActorLocation() + Source->GetActorRotation().RotateVector(Source->GetCameraBoom()->SocketOffset) + Source->GetControlRotation().Vector() * Range;
	        const FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(Start, End);
	        const FTransform Transform = FTransform(Rotation, Start);

			ARangeBaseProjectile* Projectile = CastChecked<AEfhorisGameMode>(GetWorld()->GetAuthGameMode())
				->SpawnActorDeferred<ARangeBaseProjectile>(ProjectileClass, Transform, GetOwningActorFromActorInfo(), Source,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			Projectile->SetDamageEffectSpecHandle(CreateDamageEffect(DamageEffect, DamageInfo));
			Projectile->SetRange(Range);
			Projectile->FinishSpawning(Transform);
		}
}
