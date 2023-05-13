#include "EGA_AIPotion.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjetHiver/Characters/Mercenary.h"
#include "ProjetHiver/Characters/AttributeSets/HealthAttributeSet.h"
#include "ProjetHiver/Characters/AttributeSets/PotionAttributeSet.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/Items/ReplicatedStaticMeshActor.h"

UEGA_AIPotion::UEGA_AIPotion()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    const FGameplayTag PotionTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Potion"));

    AbilityTags.AddTag(PotionTag);
    ActivationOwnedTags.AddTag(PotionTag);

    BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action")));

    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsAiming")));

    ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));

    CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Move.Block")));
}

void UEGA_AIPotion::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
        {
            if (IsValid(MontageTask)) MontageTask->EndTask();
            if (IsValid(GameplayEventTask)) GameplayEventTask->EndTask();

            if (HasAuthority(&CurrentActivationInfo))
            {
               // Character->SetLeftWeaponVisibility(false);
                PotionBottleActor->Destroy();
            }
        });

    if (!CommitCheck(Handle, ActorInfo, ActivationInfo))
        if (HasAuthority(&CurrentActivationInfo))
            CancelAbility(Handle, ActorInfo, ActivationInfo, true);

    Character = CastChecked<AMercenary>(ActorInfo->OwnerActor);

    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DrinkingMontage, MontagePlayingRate);
    MontageTask->OnCompleted.AddDynamic(this, &UEGA_AIPotion::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UEGA_AIPotion::OnMontageCompleted);
    MontageTask->OnCancelled.AddDynamic(this, &UEGA_AIPotion::OnMontageCompleted);

    GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.DrinkPotion")));
    GameplayEventTask->EventReceived.AddDynamic(this, &UEGA_AIPotion::OnGameplayEventReceived);

    //Character->SetLeftWeaponVisibility(true);

    const FName SocketName = FName("PotionSocket");
    const auto Transform = Character->GetMesh()->GetSocketTransform(SocketName);

    PotionBottleActor = CastChecked<AEfhorisGameMode>(GetWorld()->GetAuthGameMode())->SpawnActor<AReplicatedStaticMeshActor>(PotionBottle.Get(), Transform);
    PotionBottleActor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), SocketName);

    MontageTask->ReadyForActivation();
    GameplayEventTask->ReadyForActivation();
}

bool UEGA_AIPotion::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    const AMercenary* Mercenary = CastChecked<AMercenary>(ActorInfo->OwnerActor);

    return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) &&
        Mercenary->GetPotionCharge() > 0.f &&
        Mercenary->GetHealth() < Mercenary->GetMaxHealth();
}

void UEGA_AIPotion::OnMontageCompleted()
{
    if (HasAuthority(&CurrentActivationInfo))
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_AIPotion::OnGameplayEventReceived(FGameplayEventData GameplayEventData)
{
    CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);

    FGameplayEffectSpecHandle GESpecHandle = MakeOutgoingGameplayEffectSpec(HealEffect, GetAbilityLevel());
    const float MaxHealth = GetAbilitySystemComponentFromActorInfo()->GetNumericAttribute(UHealthAttributeSet::GetMaxHealthAttribute());
    GESpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.Health")), MaxHealth * HealCoefficient);
    ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, GESpecHandle);
}
