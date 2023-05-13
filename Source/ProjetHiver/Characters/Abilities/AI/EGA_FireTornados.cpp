#include "ProjetHiver/Characters/Abilities/AI/EGA_FireTornados.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/Characters/Abilities/Projectiles/RangeBaseProjectile.h"

UEGA_FireTornados::UEGA_FireTornados()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.DragonBoss.FireTornados"));

	AbilityTags.AddTag(AttackTag);

	ActivationOwnedTags.AddTag(AttackTag);

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));
}

void UEGA_FireTornados::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
		{
			if (IsValid(MontageTask)) MontageTask->EndTask();
			if (IsValid(GameplayEventTask)) GameplayEventTask->EndTask();
		});

	CommitAbility(Handle, ActorInfo, ActivationInfo);

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage, MontagePlayingRate);
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_FireTornados::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_FireTornados::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_FireTornados::OnMontageCompleted);

	GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.SpawnProjectile")));
	GameplayEventTask->EventReceived.AddDynamic(this, &UEGA_FireTornados::OnGameplayEventReceived);

	MontageTask->ReadyForActivation();
	GameplayEventTask->ReadyForActivation();
}

void UEGA_FireTornados::OnMontageCompleted()
{
	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_FireTornados::OnGameplayEventReceived(FGameplayEventData GameplayEventData)
{
	if (HasAuthority(&CurrentActivationInfo))
		if (ABaseCharacter* Source = Cast<ABaseCharacter>(GetOwningActorFromActorInfo()))
		{
			FVector Start = Source->GetMesh()->GetSocketLocation(SocketToSpawnProjectile);
			Start.Z = 0;
			const FVector End = Start + Source->GetActorForwardVector() * Range;

			FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(Start, End).Vector().RotateAngleAxis(-SpreadAngleInDegree / 2, FVector::UpVector).Rotation();

			for (int i = 0; i < NbProjectiles; ++i)
			{
				Rotation = Rotation.Add(0, SpreadAngleInDegree / NbProjectiles, 0);

				FTransform Transform = FTransform(Rotation, Start);

				ARangeBaseProjectile* Projectile = CastChecked<AEfhorisGameMode>(GetWorld()->GetAuthGameMode())
					->SpawnActorDeferred<ARangeBaseProjectile>(ProjectileClass, Transform, GetOwningActorFromActorInfo(), Source,
						ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				Projectile->SetDamageEffectSpecHandle(CreateDamageEffect(DamageEffect, DamageInfo));
				Projectile->SetRange(Range);
				Projectile->FinishSpawning(Transform);
			}


		}
}
