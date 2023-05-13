#include "EGA_DragonMeteor.h"


#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjetHiver/Characters/DragonBoss.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/Characters/Abilities/Projectiles/RangeBaseProjectile.h"

UEGA_DragonMeteor::UEGA_DragonMeteor()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.DragonBoss.Meteor"));

	AbilityTags.AddTag(AttackTag);

	ActivationOwnedTags.AddTag(AttackTag);

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));
}

void UEGA_DragonMeteor::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
		{
			if (IsValid(MontageTask)) MontageTask->EndTask();
			if (IsValid(GameplayEventTask)) GameplayEventTask->EndTask();
			if (IsValid(WaitDelayTask)) WaitDelayTask->EndTask();
		});

	CommitAbility(Handle, ActorInfo, ActivationInfo);

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage, MontagePlayingRate);
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_DragonMeteor::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_DragonMeteor::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_DragonMeteor::OnMontageCompleted);

	GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.SpawnProjectiles")));
	GameplayEventTask->EventReceived.AddDynamic(this, &UEGA_DragonMeteor::OnGameplayEventReceived);

	MontageTask->ReadyForActivation();
	GameplayEventTask->ReadyForActivation();

	WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, Cooldown);
	WaitDelayTask->OnFinish.AddDynamic(this, &UEGA_DragonMeteor::OnWaitFinished);
	WaitDelayTask->ReadyForActivation();
	bCanActivate = true;
}

void UEGA_DragonMeteor::OnMontageCompleted()
{
	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_DragonMeteor::OnWaitFinished()
{
	bCanActivate = true;
}

void UEGA_DragonMeteor::OnGameplayEventReceived(FGameplayEventData GameplayEventData)
{
	if (HasAuthority(&CurrentActivationInfo) && bCanActivate)
		if (ADragonBoss* DragonBoss = Cast<ADragonBoss>(GetOwningActorFromActorInfo()))
		{
			//Find random vector in range
			FVector ProjectileLocation = UKismetMathLibrary::RandomPointInBoundingBox(DragonBoss->GetInitialLocation(), FVector{ SpawnRadius });
			ProjectileLocation.Z = SpawnZ;

			const FTransform Transform = FTransform(ProjectileLocation);

			ARangeBaseProjectile* Projectile = CastChecked<AEfhorisGameMode>(GetWorld()->GetAuthGameMode())
				->SpawnActorDeferred<ARangeBaseProjectile>(ProjectileClass, Transform, GetOwningActorFromActorInfo(), DragonBoss,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			Projectile->SetDamageEffectSpecHandle(CreateDamageEffect(DamageEffect, DamageInfo));
			Projectile->SetRange(SpawnZ);
			Projectile->FinishSpawning(Transform);
			bCanActivate = false;

			WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, Cooldown);
			WaitDelayTask->OnFinish.AddDynamic(this, &UEGA_DragonMeteor::OnWaitFinished);
			WaitDelayTask->ReadyForActivation();
		}
}
