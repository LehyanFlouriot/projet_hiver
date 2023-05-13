// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjetHiver/Characters/Abilities/AI/EGA_MeleeLightDistanceAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/BaseNPC.h"
#include "ProjetHiver/Characters/Abilities/Projectiles/RangeBaseProjectile.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"

void UEGA_MeleeLightDistanceAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_MeleeLightDistanceAttack::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_MeleeLightDistanceAttack::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_MeleeLightDistanceAttack::OnMontageCompleted);

	GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.SpawnProjectile")));
	GameplayEventTask->EventReceived.AddDynamic(this, &UEGA_MeleeLightDistanceAttack::OnGameplayEventReceived);

	MontageTask->ReadyForActivation();
	GameplayEventTask->ReadyForActivation();
}

void UEGA_MeleeLightDistanceAttack::OnGameplayEventReceived(FGameplayEventData GameplayEventData)
{
	if (HasAuthority(&CurrentActivationInfo))
		if (ABaseNPC* Source = Cast<ABaseNPC>(GetOwningActorFromActorInfo()))
		{
			constexpr float SpawnOffset = 50.f;
			const FVector Start = Source->GetActorLocation() + Source->GetActorForwardVector() * SpawnOffset;
			const FVector End = Source->GetTarget()->GetActorLocation();
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
