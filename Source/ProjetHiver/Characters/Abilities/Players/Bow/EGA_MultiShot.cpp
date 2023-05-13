#include "ProjetHiver/Characters/Abilities/Players/Bow/EGA_MultiShot.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Characters/Abilities/Projectiles/RangeBumpProjectile.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"

UEGA_MultiShot::UEGA_MultiShot()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.Bow.MultiShot"));

	AbilityTags.AddTag(AttackTag);

	ActivationOwnedTags.AddTag(AttackTag);

	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Jump")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Interact")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Potion")));

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));
}

void UEGA_MultiShot::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	InitCooldownOnWidget();

	SlowEffectHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,
		MakeOutgoingGameplayEffectSpec(SlowEffect));

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage, MontagePlayingRate);
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_MultiShot::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_MultiShot::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_MultiShot::OnMontageCompleted);

	GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.SpawnProjectile")));
	GameplayEventTask->EventReceived.AddDynamic(this, &UEGA_MultiShot::OnGameplayEventReceived);

	MontageTask->ReadyForActivation();
	GameplayEventTask->ReadyForActivation();
}

void UEGA_MultiShot::OnMontageCompleted()
{
	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_MultiShot::OnGameplayEventReceived(FGameplayEventData GameplayEventData)
{
	if (HasAuthority(&CurrentActivationInfo))
		if (APlayerCharacter* Source = Cast<APlayerCharacter>(GetOwningActorFromActorInfo()))
		{
			const FVector Start = Source->GetMesh()->GetSocketLocation(SocketToSpawnProjectile);
			const FVector End = Source->GetActorLocation() + Source->GetActorRotation().RotateVector(Source->GetCameraBoom()->SocketOffset) + Source->GetControlRotation().Vector() * Range;

			FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(Start, End).Vector().RotateAngleAxis(-SpreadAngleInDegree / 2, FVector::UpVector).Rotation();

			for(int i = 0; i < NbProjectiles; ++i)
			{
				Rotation = Rotation.Add(0, SpreadAngleInDegree / NbProjectiles, 0);

				FTransform Transform = FTransform(Rotation, Start);

				ARangeBumpProjectile* Projectile = CastChecked<AEfhorisGameMode>(GetWorld()->GetAuthGameMode())
					->SpawnActorDeferred<ARangeBumpProjectile>(ProjectileClass, Transform, GetOwningActorFromActorInfo(), Source,
						ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				Projectile->SetDamageEffectSpecHandle(CreateDamageEffect(DamageEffect, DamageInfo));
				Projectile->SetRange(Range);
				Projectile->SetBumpForce(BumpForce);
				Projectile->SetUpCoefficient(UpCoefficient);

				Projectile->FinishSpawning(Transform);
			}
		}
}
