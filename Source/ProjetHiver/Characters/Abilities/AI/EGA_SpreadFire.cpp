#include "EGA_SpreadFire.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/Characters/Abilities/Projectiles/RangeBaseProjectile.h"

UEGA_SpreadFire::UEGA_SpreadFire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.DragonBoss.SpreadFireGrounded"));

	AbilityTags.AddTag(AttackTag);

	ActivationOwnedTags.AddTag(AttackTag);

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));
}

void UEGA_SpreadFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
		{
			if (IsValid(MontageTask)) MontageTask->EndTask();
			if (IsValid(SpawnProjectilesEventTask)) SpawnProjectilesEventTask->EndTask();
		});

	CommitAbility(Handle, ActorInfo, ActivationInfo);

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage, MontagePlayingRate);
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_SpreadFire::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_SpreadFire::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_SpreadFire::OnMontageCompleted);

	SpawnProjectilesEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.SpawnProjectiles")));
	SpawnProjectilesEventTask->EventReceived.AddDynamic(this, &UEGA_SpreadFire::OnSpawnProjectilesEventReceived);

	MontageTask->ReadyForActivation();
	SpawnProjectilesEventTask->ReadyForActivation();
}

void UEGA_SpreadFire::OnMontageCompleted()
{
	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_SpreadFire::OnSpawnProjectilesEventReceived(FGameplayEventData GameplayEventData)
{
	if (HasAuthority(&CurrentActivationInfo))
		if (ABaseCharacter* Source = Cast<ABaseCharacter>(GetOwningActorFromActorInfo()))
		{
			const FVector Start = Source->GetMesh()->GetSocketLocation(SocketToSpawnProjectile);
			const FRotator Rotation = Source->GetMesh()->GetSocketTransform(SocketToSpawnProjectile).Rotator();

			const FTransform Transform = FTransform(Rotation, Start);

			ARangeBaseProjectile* Projectile = CastChecked<AEfhorisGameMode>(GetWorld()->GetAuthGameMode())
				->SpawnActorDeferred<ARangeBaseProjectile>(ProjectileClass, Transform, GetOwningActorFromActorInfo(), Source,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			Projectile->SetDamageEffectSpecHandle(CreateDamageEffect(DamageEffect, DamageInfo));
			Projectile->SetRange(Range);
			Projectile->FinishSpawning(Transform);
			
		}
}
