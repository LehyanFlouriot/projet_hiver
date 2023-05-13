#include "HealthAttributeSet.h"
#include "AkComponent.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/BaseNPC.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/AI/PlayersStatsMeasuringComponent.h"
#include "ProjetHiver/BDD/UserScoreboardStatistic.h"
#include "ProjetHiver/Characters/DragonBoss.h"
#include "ProjetHiver/Characters/Dummy.h"
#include "ProjetHiver/Characters/Mercenary.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"

void UHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
	}
}

void UHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();

	AActor* TargetActor = nullptr;
	AController* TargetController = nullptr;
	ABaseCharacter* TargetCharacter = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		TargetCharacter = Cast<ABaseCharacter>(TargetActor);
	}

	// Get the Source actor
	AActor* SourceActor = nullptr;
	AController* SourceController = nullptr;
	ABaseCharacter* SourceCharacter = nullptr;
	if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
	{
		SourceActor = Source->AbilityActorInfo->AvatarActor.Get();
		SourceController = Source->AbilityActorInfo->PlayerController.Get();
		if (SourceController == nullptr && SourceActor != nullptr)
		{
			if (APawn* Pawn = Cast<APawn>(SourceActor))
			{
				SourceController = Pawn->GetController();
			}
		}

		// Use the controller to find the source pawn
		if (SourceController)
		{
			SourceCharacter = Cast<ABaseCharacter>(SourceController->GetPawn());
		}
		else
		{
			SourceCharacter = Cast<ABaseCharacter>(SourceActor);
		}

		// Set the causer actor based on context if it's set
		if (Context.GetEffectCauser())
		{
			SourceActor = Context.GetEffectCauser();
		}
	}

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamageDone = GetDamage();
		SetDamage(0.f);

		if (LocalDamageDone > 0.f)
		{
			bool WasAlive = true;

			if (TargetCharacter) WasAlive = TargetCharacter->IsAliveHealth();

			const float NewHealth = GetHealth() - LocalDamageDone;

			//Inflict damage on non dummy
			if (ADummy* Dummy = Cast<ADummy>(TargetCharacter); IsValid(Dummy)) {
				Dummy->AddDamage(LocalDamageDone);
			}
			else {
				SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
			}

			if (WasAlive)
			{
				if (TargetCharacter)
				{
					if(UEfhorisGameInstance* GameInstance = GetWorld()->GetGameInstance<UEfhorisGameInstance>(); IsValid(GameInstance)){
					//			-----------------------------
						FAkAudioDevice::Get()->PostEvent(GameInstance->HitSound, TargetCharacter);
					}

					// Collect the damage taken stat for the AI director
					if (TargetCharacter->IsA<APlayerCharacter>())
						if (APlayerCharacter* Player = Cast<APlayerCharacter>(TargetCharacter); Player->HasAuthority())
							Player->GetPlayersStatsMeasuringComponent()->Authority_AddDamageTaken(static_cast<double>(LocalDamageDone));

					//			-----------------------------
					if (SourceActor != TargetActor)
					{
						ABaseNPC* TargetNPC = Cast<ABaseNPC>(TargetCharacter);
						//Calculate Threat for Enemies
						//Player or Mercenary hits a NPC
						if (SourceCharacter) {
							if (TargetNPC && !TargetNPC->IsAlly()) {
								TargetNPC->AddThreat(SourceCharacter, 10 * LocalDamageDone);
							}
						}

						//Calculate Threat for Mercenaries
						//Player hits an enemy
						if (APlayerCharacter* Player = Cast<APlayerCharacter>(SourceCharacter)) {
							if (TargetCharacter) {
								for (AMercenary* Mercenary  : Player->GetMercenaries()) {
									if (IsValid(Mercenary)) {
										Mercenary->AddThreat(TargetCharacter, 10 * LocalDamageDone);
									}
								}
							}

							if (AEfhorisPlayerState* PlayerState = Player->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState))
							{
								PlayerState->IncreaseStatistic(EUserScoreboardStatistic::DamageDealt, LocalDamageDone);

								if (!TargetCharacter->IsAliveHealth())
									if (TargetCharacter->IsA<AEnemy>())
										PlayerState->IncreaseStatistic(EUserScoreboardStatistic::EnemiesKilled, 1);
							}
						}
						//Enemy hits a Player or a Mercenary
						if (SourceCharacter) {
							if (APlayerCharacter* Player = Cast<APlayerCharacter>(TargetActor)) {
								for (AMercenary* Mercenary : Player->GetMercenaries()) {
									if (IsValid(Mercenary)) {
										Mercenary->AddThreat(SourceCharacter, 10 * LocalDamageDone);
									}
								}

								if (AEfhorisPlayerState* PlayerState = Player->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState))
									PlayerState->IncreaseStatistic(EUserScoreboardStatistic::DamageReceived, LocalDamageDone);
							}
							if (AMercenary* Mercenary = Cast<AMercenary>(TargetActor))
							{
								Mercenary->AddThreat(SourceCharacter, 10 * LocalDamageDone);
							}
						}
						
						// Show damage number for the Source player
						if (ABaseCharacter* Target = Cast<ABaseCharacter>(TargetActor)) {
							if (const AEfhorisPlayerController* PC = Cast<AEfhorisPlayerController>(SourceController))
							{
								PC->Client_ShowDamageNumber(Target, LocalDamageDone);
								PC->Client_OnHitShake(FMath::Clamp(LocalDamageDone/20.0f,0,2));
								if(const ADragonBoss* Boss = Cast<ADragonBoss>(Target); IsValid(Boss) && !Boss->GetIsInCombat())
								{
									GetWorld()->GetAuthGameMode<AEfhorisGameMode>()->EnterBossCombat();
								}
							}
						}
					}

					if (!TargetCharacter->IsAliveHealth()) TargetCharacter->Die();
				}
				// We can handle damage to props in a else statement here.
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Should never be used because health will be reduced through damage received
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}

void UHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, Health, OldHealth);
}

void UHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, MaxHealth, OldMaxHealth);
}
