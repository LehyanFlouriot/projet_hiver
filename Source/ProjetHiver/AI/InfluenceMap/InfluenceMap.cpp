#include "InfluenceMap.h"

bool UInfluenceMap::IsInBound(const int& I) const noexcept {
	return 0 <= I && I < Data.Num();
}

bool UInfluenceMap::IsInBound(const int& X, const int& Y) const noexcept {
	return 0 <= X && X < Size && 0 <= Y && Y < Size;
}

float UInfluenceMap::GetDataUnsafe(const int& I) const {
	return Data[I];
}

float UInfluenceMap::GetDataUnsafe(const int& X, const int& Y) const {
	return Data[Size * X + Y];
}

FIntVector2 UInfluenceMap::XYFromI(const int& I) const {
	const int X = I / Size;
	const int Y = I % Size;
	return FIntVector2(X,Y);
}

int UInfluenceMap::IFromXY(const FIntVector2& XY) const {
	return Size * XY.X + XY.Y;
}

float UInfluenceMap::DecayFunction(const float Value, const float& Decay)
{
	return FMath::Abs(Value) > 0.01f ? Decay * Value : 0.0f;
}

void UInfluenceMap::Map(const TFunctionRef<void(float& A)> F) {
	for (int I = 0; I < Data.Num(); ++I) {
		F(Data[I]);
	}
}

void UInfluenceMap::MapWithOther(const TFunctionRef<void(float& A, float& B) > F, UInfluenceMap* Other) {
	for (int I = 0; I < Data.Num(); ++I) {
		F(Data[I], Other->Data[I]);
	}
}

void UInfluenceMap::MapWithOther(const TFunctionRef<void(float& A, const float& B) > F, const UInfluenceMap* Other) {
	for (int I = 0; I < Data.Num(); ++I) {
		F(Data[I], Other->Data[I]);
	}
}

//TileSize should be initialized first. Done in Init of UTemplateManager
void UInfluenceMap::Init(const int& Size_, const int& X, const int& Y,const float DecayRate_) {
	Size = Size_;
	DecayRate = FMath::Clamp(DecayRate_,0.0f,1.0f);
	Data.Init(0.0f, Size * Size);
	GlobalPosition = FIntVector2(X, Y);
	StartWorldLocation = FVector2D(TileSize * X - TileSize/2, TileSize * Y - TileSize/2);
}

void UInfluenceMap::Init(const int& Size_, const int& TMapX, const int TMapY, const int StartX, const int StartY, const float DecayRate_) {
	Init(Size_, TMapX, TMapY, DecayRate_);
	const FVector2D Offset = FVector2D(StartX * CellSize, StartY * CellSize);
	StartWorldLocation += Offset;
}

void UInfluenceMap::Tick(const float& DeltaTime) {
	Decay(DeltaTime);
}

UInfluenceMap* UInfluenceMap::CopyMap() const {
	UInfluenceMap* Copy = NewObject<UInfluenceMap>();
	Copy->Init(Size, GlobalPosition.X, GlobalPosition.Y,DecayRate);
	Copy->StartWorldLocation = StartWorldLocation;
	for (int I = 0; I < Data.Num(); I++) {
		Copy->Data[I] = Data[I];
	}
	return Copy;
}

// OPERATORS

bool UInfluenceMap::operator==(const UInfluenceMap* Other) {
	if (Size != Other->Size) return false;
	for(int I =0; I < Data.Num(); I++) {
		if(Data[I] != Other->Data[I]) return false;
	}
	return true;
}

void UInfluenceMap::operator+(const UInfluenceMap* Other) {
	if (Size != Other->GetSize()) return;
	MapWithOther([](float& A, const float& B) {A += B;}, Other);
}

void UInfluenceMap::operator-(const UInfluenceMap* Other) {
	if (Size != Other->GetSize()) return;
	MapWithOther([](float& A, const float& B) {A -= B; }, Other);
}

void UInfluenceMap::operator*(const UInfluenceMap* Other) {
	if (Size != Other->GetSize()) return;
	MapWithOther([](float& A, const float& B) {A *= B; }, Other);
}

void UInfluenceMap::Add(const UInfluenceMap* Other, const float& Magnitude) {
	if (Size != Other->GetSize()) return;
	if (Magnitude == 1.0f) {
		*this + Other;
		return;
	}
	if (Magnitude == -1.0f) {
		*this - Other;
		return;
	}
	MapWithOther([Magnitude](float& A, const float& B) {A += B * Magnitude; }, Other);
}

void UInfluenceMap::Multiply(const float& Magnitude) {
	Map([Magnitude](float& A) {A *= Magnitude; });
}

void UInfluenceMap::AddAt(const UInfluenceMap* Other, const FIntVector2& StartXY, const int& Magnitude) {
	const int EndX = StartXY.X + Other->Size;
	const int EndY = StartXY.Y + Other->Size;
	const int MinX = FMath::Max(0, StartXY.X);
	const int MaxX = FMath::Min(EndX, Size);
	const int MinY = FMath::Max(0, StartXY.Y);
	const int MaxY = FMath::Min(EndY, Size);
	for (int X = MinX; X < MaxX; X++) {
		for (int Y = MinY; Y < MaxY; Y++) {
			const int OtherX = X - StartXY.X;
			const int OtherY = Y - StartXY.Y;
			SetData(X, Y, GetDataUnsafe(X, Y) + Other->GetDataUnsafe(OtherX, OtherY) * Magnitude);
		}
	}
}

void UInfluenceMap::AddAtCenter(const UInfluenceMap* Other, const FIntVector2& CenterXY, const int& Magnitude) {
	const int StartX = CenterXY.X - Other->Size / 2;
	const int StartY = CenterXY.Y - Other->Size / 2;
	const FIntVector2 StartXY = FIntVector2(StartX, StartY);
	AddAt(Other, StartXY, Magnitude);
}

void UInfluenceMap::ApplyAt(const UInfluenceMap* Other, const FIntVector2& StartXY) {
	UInfluenceMap* Result = Other->Size >= Size ? this : Extract(FMath::Min(Other->Size,Size), StartXY.X, StartXY.Y);
	Result->MapWithOther([](float& A, const float& B) {A *= B; }, Other);
	Data = Result->Data;
	Size = Result->Size;
	StartWorldLocation = Result->StartWorldLocation;
}

void UInfluenceMap::ApplyAtCenter(const UInfluenceMap* Other, const FIntVector2& CenterXY) {
	const int StartX = CenterXY.X - Other->Size / 2;
	const int StartY = CenterXY.Y - Other->Size / 2;
	const FIntVector2 StartXY = FIntVector2(StartX, StartY);
	ApplyAt(Other, StartXY);
}

UInfluenceMap* UInfluenceMap::Extract(const int& ExtractionSize, const int& StartX, const int& StartY) const {
	UInfluenceMap* Result = NewObject<UInfluenceMap>();
	Result->Init(ExtractionSize, GlobalPosition.X, GlobalPosition.Y, StartX, StartY, DecayRate);
	const int EndX = StartX + ExtractionSize;
	const int EndY = StartY + ExtractionSize;
	const int MinX = FMath::Max(0, StartX);
	const int MaxX = FMath::Min(EndX, Size);
	const int MinY = FMath::Max(0, StartY);
	const int MaxY = FMath::Min(EndY, Size);
	for (int X = MinX; X < MaxX; X++) {
		for (int Y = MinY; Y < MaxY; Y++) {
			const int OtherX = X - StartX;
			const int OtherY = Y - StartY;
			Result->SetData(OtherX, OtherY, GetDataUnsafe(X, Y));
		}
	}
	return Result;
}

//Non directional
FIntVector2 UInfluenceMap::FindHighest() const {
	if (Size == 0) { return FIntVector2(-1,-1); }

	float MaxValue = GetDataUnsafe(0);
	TArray<int> PotentialMaximums;

	for(int I = 0; I < Data.Num(); ++I) {
		const float Value = GetDataUnsafe(I);
		if (MaxValue < Value) {
			MaxValue = Value;
			PotentialMaximums.Empty();
			PotentialMaximums.Add(I);
		}
		else if (MaxValue == Value) {
			PotentialMaximums.Add(I);
		}
	}

	const int Max = PotentialMaximums[FMath::RandRange(0, PotentialMaximums.Num() - 1)];
	return XYFromI(Max);
}

//Directional
FIntVector2 UInfluenceMap::FindHighestAbsolute() const {
	if (Size == 0) { return FIntVector2(-1, -1); }
	int MaxI = 0;
	float MaxValue = FMath::Abs(Data[0]);
	for (int I = 1; I < Data.Num(); I++) {
		const float AbsValue = FMath::Abs(Data[I]);
		if (MaxValue < AbsValue) {
			MaxI = I;
			MaxValue = AbsValue;
		}
	}
	return XYFromI(MaxI);
}

void UInfluenceMap::Normalize() {
	const FIntVector2 Highest = FindHighestAbsolute();
	const float Magnitude = FMath::Abs(GetData(Highest.X, Highest.Y));
	if (Magnitude != 0.0f) {
		Multiply(1.0f / Magnitude);
	}
}

void UInfluenceMap::Decay(const float& DeltaTime) {
	const float DecayMultiplier = FMath::Pow(DecayRate, DeltaTime*100);
	Map([DecayMultiplier](float& A) {A = DecayFunction(A,DecayMultiplier); });
}

// GETTERS && SETTERS

float UInfluenceMap::GetData(const int& I) const {
	return IsInBound(I) ? Data[I] : 0.0f;
}

float UInfluenceMap::GetData(const int& X, const int& Y) const {
	return GetData(Size * X + Y);
}

float UInfluenceMap::GetData(const FIntVector2& XY) const {
	return GetData(XY.X,XY.Y);
}

void UInfluenceMap::SetData(const int& I, const float& Value) {
	if (IsInBound(I)) {
		Data[I] = Value;
	}
}

void UInfluenceMap::SetData(const int& X, const int& Y, const float& Value) {
	SetData(Size * X + Y, Value);
}

void UInfluenceMap::SetData(const FIntVector2& XY, const float& Value) {
	SetData(XY.X, XY.Y, Value);
}

//Returns the world location corresponding to the Cell XY of the Imap
FVector2D UInfluenceMap::ToWorldTransform(const FIntVector2& XY) const {
	const FVector2D Offset = FVector2D(CellSize * XY.X, CellSize * XY.Y);
	return StartWorldLocation + Offset;
}

//Returns the Imap cell coordinates (XY) corresponding to the world Location. XY can be out of bounds.
FIntVector2 UInfluenceMap::ToInMapXY(const FVector& Location) const {
	const int IntX = FMath::Floor((Location.X- StartWorldLocation.X) / CellSize);
	const int IntY = FMath::Floor((Location.Y - StartWorldLocation.Y) / CellSize);
	return FIntVector2(IntX, IntY);
}

int UInfluenceMap::GetSize() const noexcept {
	return Size;
}

float UInfluenceMap::GetCellSize() noexcept {
	return CellSize;
}

//If argument is nullptr, will returns corners based on the default Anchor
TArray<FVector> UInfluenceMap::GetCorners(const FVector* Anchor_) const {
	TArray<FVector> Corners;
	Corners.Reserve(4);
	const FVector2D NewAnchor = Anchor_ ? FVector2D(Anchor_->X,Anchor_->Y) : StartWorldLocation;
	const float Length = Size * CellSize;
	Corners.Add(FVector(NewAnchor.X,NewAnchor.Y,0));
	Corners.Add(FVector(NewAnchor.X + Length, NewAnchor.Y,0));
	Corners.Add(FVector(NewAnchor.X, NewAnchor.Y + Length,0));
	Corners.Add(FVector(NewAnchor.X + Length, NewAnchor.Y + Length,0));
	return Corners;
}

//Returns the corners of the Imap as if Location was the bottom left corner
TArray<FVector> UInfluenceMap::GetCornersOffsetFromStart(const FVector& Location) const {
	return GetCorners(&Location);
}

//Returns the corners of the Imap as if Location was the center
TArray<FVector> UInfluenceMap::GetCornersOffsetFromCenter(const FVector& Location) const {
	const float Delta = Size * CellSize / 2;
	return GetCornersOffsetFromStart(FVector(Location.X -Delta, Location.Y - Delta,0));
}

void UInfluenceMap::Draw(const UWorld* World, const float DeltaSeconds) const {
	UInfluenceMap* Copy = CopyMap();
	Copy->Normalize();
	const float DisplaySize = CellSize / 4;
	const FVector Extent = FVector(DisplaySize, DisplaySize, 0);

	for(int I =0; I< Data.Num(); I++) {
		const FIntVector2 XY = XYFromI(I);
		const FVector Center = FVector(CellSize * XY.X + DisplaySize + StartWorldLocation.X, CellSize * XY.Y + DisplaySize + StartWorldLocation.Y, DisplayHeight);
		const float Magnitude = Copy->Data[I];
		FColor Color;
		if (Magnitude<0){
			Color = FColor(255, 0, 0, -Magnitude*255);
		}
		else {
			Color = FColor(0, 255, 0, Magnitude*255);
		}
		if (FMath::Abs(Magnitude)>0.f) {
			DrawDebugSolidBox(World, Center, Extent, Color, false, DeltaSeconds);
			//DrawDebugPoint(World, Center, 12.0f, Color,false,DeltaSeconds); //Much less lag but no alpha
		}
	}
}