#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ProjetHiver/Items/ItemStats.h"
#include "WeaponKitRow.generated.h"

class AReplicatedStaticMeshActor;
class UBlendSpace;
class UEfhorisGameplayAbility;

USTRUCT(BlueprintType)
struct PROJETHIVER_API FWeaponKitRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Name"))
	FText Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Description"))
	FText Description;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Base Attack Combo"))
	TArray<TSubclassOf<UEfhorisGameplayAbility>> BaseAttackCombo;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Right Click Ability", MakeStructureDefaultValue="None"))
	TSubclassOf<UEfhorisGameplayAbility> RightClickAbility;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Idle Animation", MakeStructureDefaultValue = "None"))
	TObjectPtr<UAnimSequence> IdleAnimation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Walking Blend Space", MakeStructureDefaultValue = "None"))
	TObjectPtr<UBlendSpace> WalkingBlendSpace;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Jump Animation", MakeStructureDefaultValue = "None"))
	TObjectPtr<UAnimSequence> JumpAnimation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Primary Abilities"))
	TArray<TSubclassOf<UEfhorisGameplayAbility>> PrimaryAbilities;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Secondary Abilities"))
	TArray<TSubclassOf<UEfhorisGameplayAbility>> SecondaryAbilities;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Dodge Ability", MakeStructureDefaultValue="None"))
	TSubclassOf<UEfhorisGameplayAbility> DodgeAbility;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Icon", MakeStructureDefaultValue="None"))
	TObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Right Hand Actor", MakeStructureDefaultValue="None"))
	TSubclassOf<AReplicatedStaticMeshActor> RightHandActor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Left Hand Actor", MakeStructureDefaultValue="None"))
	TSubclassOf<AReplicatedStaticMeshActor> LeftHandActor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Loot Actor", MakeStructureDefaultValue="None"))
	TSubclassOf<AReplicatedStaticMeshActor> LootActor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Base Item Stats"))
	FItemStats BaseItemStats;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Base Buy Value", MakeStructureDefaultValue="0.000000"))
	int BaseBuyValue = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Base Sell Value", MakeStructureDefaultValue="0.000000"))
	int BaseSellValue = 0;
};
