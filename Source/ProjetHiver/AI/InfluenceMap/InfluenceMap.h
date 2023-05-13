#pragma once

#include "CoreMinimal.h"
#include "InfluenceMap.generated.h"

class UTemplateManager;

UCLASS(Transient)
class PROJETHIVER_API UInfluenceMap final : public UObject
{
	GENERATED_BODY()

	friend class UTemplateManager;

private:
	UPROPERTY(Transient)
	TArray<float> Data; //Array of Size*Size elements

	UPROPERTY(Transient)
	int Size;

	UPROPERTY(Transient)
	FIntVector2 GlobalPosition; //Position of the Imap in the UInfluenceMapManager grid

	UPROPERTY(Transient)
	FVector2D StartWorldLocation; //Location of the (0,0) corner in the world

	inline static float TileSize = 6000.0f;

	inline static float CellSize;

	inline static float DisplayHeight = 150.0f;

	UPROPERTY(Transient)
	float DecayRate;

	bool IsInBound(const int& I) const noexcept;
	bool IsInBound(const int& X, const int& Y) const noexcept;

	float GetDataUnsafe(const int& I) const;
	float GetDataUnsafe(const int& X, const int& Y) const;

	FIntVector2 XYFromI(const int& I) const;
	int IFromXY(const FIntVector2& XY) const;

	UFUNCTION()
	static float DecayFunction(const float Value, const float& Decay);

	//Map unary function F on Data
	void Map(const TFunctionRef<void(float& A)> F);

	//Map binary function F on (Data, Other->Data)
	void MapWithOther(const TFunctionRef<void(float& A, float& B) > F, UInfluenceMap* Other);

	//Map binary function F on (Data, Other->Data) AND Other is const
	void MapWithOther(const TFunctionRef<void(float& A, const float& B) > F, const UInfluenceMap* Other);

public:
	void Init(const int& Size_, const int& X = 0, const int& Y = 0,const float DecayRate_ = 0.95f);
	void Init(const int& Size_, const int& TMapX, const int TMapY, const int StartX, const int StartY, const float DecayRate_ = 0.95f);

	UFUNCTION()
	void Tick(const float& DeltaTime);

	UFUNCTION()
	UInfluenceMap* CopyMap() const ;

	//OPERATORS
	bool operator==(const UInfluenceMap* Other);
	void operator+(const UInfluenceMap* Other);
	void operator-(const UInfluenceMap* Other);
	void operator*(const UInfluenceMap* Other);

	UFUNCTION()
	void Add(const UInfluenceMap* Other, const float& Magnitude = 1.0f);

	UFUNCTION()
	void Multiply(const float& Magnitude = 1.0f);

	void AddAt(const UInfluenceMap* Other, const FIntVector2& StartXY, const int& Magnitude = 1.0f);

	void AddAtCenter(const UInfluenceMap* Other, const FIntVector2& CenterXY, const int& Magnitude = 1.0f);

	void ApplyAt(const UInfluenceMap* Other, const FIntVector2& StartXY);

	void ApplyAtCenter(const UInfluenceMap* Other, const FIntVector2& CenterXY);

	UInfluenceMap* Extract(const int& ExtractionSize, const int& StartX, const int& StartY) const;

	UFUNCTION()
	FIntVector2 FindHighest() const;

	UFUNCTION()
	FIntVector2 FindHighestAbsolute() const;

	UFUNCTION()
	void Normalize();

	UFUNCTION()
	void Decay(const float& DeltaTime);

	//GETTERS && SETTERS
	float GetData(const int& I) const;
	float GetData(const int& X, const int& Y) const;
	float GetData(const FIntVector2& XY) const;

	void SetData(const int& I, const float& Value);
	void SetData(const int& X, const int& Y, const float& Value);
	void SetData(const FIntVector2& XY, const float& Value);

	FVector2D ToWorldTransform(const FIntVector2& XY) const;
	FIntVector2 ToInMapXY(const FVector& Location) const;

	int GetSize() const noexcept;
	static float GetCellSize() noexcept;

	TArray<FVector> GetCorners(const FVector* Anchor_ = nullptr) const;
	TArray<FVector> GetCornersOffsetFromStart(const FVector& Location) const;
	TArray<FVector> GetCornersOffsetFromCenter(const FVector& Location) const;

	//DEBUG
	void Draw(const UWorld* World, const float DeltaSeconds) const;
};