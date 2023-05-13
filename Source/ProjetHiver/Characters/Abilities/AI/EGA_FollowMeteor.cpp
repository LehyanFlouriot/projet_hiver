#include "EGA_FollowMeteor.h"


#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjetHiver/Characters/DragonBoss.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/Characters/Abilities/Projectiles/RangeBaseProjectile.h"

UEGA_FollowMeteor::UEGA_FollowMeteor()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.DragonBoss.FollowMeteor"));

	AbilityTags.AddTag(AttackTag);

	ActivationOwnedTags.AddTag(AttackTag);

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));
}

void UEGA_FollowMeteor::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_FollowMeteor::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_FollowMeteor::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_FollowMeteor::OnMontageCompleted);

	GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.SpawnProjectiles")));
	GameplayEventTask->EventReceived.AddDynamic(this, &UEGA_FollowMeteor::OnGameplayEventReceived);

	MontageTask->ReadyForActivation();
	GameplayEventTask->ReadyForActivation();
}

void UEGA_FollowMeteor::OnMontageCompleted()
{
	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_FollowMeteor::OnGameplayEventReceived(FGameplayEventData GameplayEventData)
{
	if (HasAuthority(&CurrentActivationInfo))
		if (ADragonBoss* DragonBoss = Cast<ADragonBoss>(GetOwningActorFromActorInfo()))
		{
			const auto SpherePos = DragonBoss->GetInitialLocation();

			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel4));

			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(DragonBoss);

			TArray<AActor*> OutActors;

			if(UKismetSystemLibrary::SphereOverlapActors(GetWorld(), SpherePos, Radius, ObjectTypes, APlayerCharacter::StaticClass(), ActorsToIgnore, OutActors))
			{
				for (AActor* Actor : OutActors)
				{
					const ABaseCharacter* HitActor = Cast<ABaseCharacter>(Actor);

					//Find random vector in range
					FVector ProjectileLocation = HitActor->GetActorLocation();
					ProjectileLocation.Z = SpawnZ;

					const FTransform Transform = FTransform(ProjectileLocation);

					ARangeBaseProjectile* Projectile = CastChecked<AEfhorisGameMode>(GetWorld()->GetAuthGameMode())
						->SpawnActorDeferred<ARangeBaseProjectile>(ProjectileClass, Transform, GetOwningActorFromActorInfo(), DragonBoss,
							ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
					Projectile->SetDamageEffectSpecHandle(CreateDamageEffect(DamageEffect, DamageInfo));
					Projectile->SetRange(SpawnZ);
					Projectile->FinishSpawning(Transform);
				}
			}
		}
}
