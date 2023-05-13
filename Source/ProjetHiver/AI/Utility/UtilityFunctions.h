#pragma once

/**
 * Functions for Utility Theory.
 */
struct PROJETHIVER_API FUtilityFunctions
{
	static constexpr float e = 2.718f;

	static constexpr float Limit = 0.0001f;

	UE_NODISCARD static FORCEINLINE bool CloserEnough(float a, float b) {
		return FMath::Abs(a - b) <= Limit;
	}

	UE_NODISCARD static FORCEINLINE float NormalisedLinear(float x, float max) {
		return x / max;
	}

	UE_NODISCARD static FORCEINLINE float NormalisedExponentiel(int x, int max, float exponentiel) {
		if (x >= max) return 1.f;
		return FMath::Pow(exponentiel, x) / FMath::Pow(exponentiel, max) - 1. / FMath::Pow(exponentiel, max);
	}

	UE_NODISCARD static FORCEINLINE float NormalisedLogarithmE(float x, float max) {
		return std::log1p(x) / std::log1p(max);
	}

	UE_NODISCARD static FORCEINLINE float NormalisedLogarithm2(float x, float max) {
		return FMath::Log2(x + 1.0f) / FMath::Log2(max + 1.0f);
	}

	UE_NODISCARD static FORCEINLINE float NormalisedLogarithm10(float x, float max) {
		return FMath::LogX(10,x + 1.0f) / FMath::LogX(10,max + 1.0f);
	}

	UE_NODISCARD static FORCEINLINE float Sigmoid(int x, float exponentiel, int decalage) {
		return 1.0f / (1.0f + FMath::Pow(exponentiel, decalage - x));
	}

	UE_NODISCARD static FORCEINLINE float LogitE(float x, float max) {
		if (CloserEnough(x, max)) return 1.f;
		else if (CloserEnough(x, 0.f)) return 0.f;
		return (FMath::Loge((x / max) / (1.0f - (x / max))) + 5.0f) / 10.0f;
	}

	UE_NODISCARD static FORCEINLINE float Logit2(float x, float max) {
		if (CloserEnough(x, max)) return 1.f;
		else if (CloserEnough(x, 0.f)) return 0.f;
		return (FMath::Log2((x / max) / (1.0f - (x / max))) + 5.0f) / 10.0f;
	}

	UE_NODISCARD static FORCEINLINE float Logit10(float x, float max) {
		if (CloserEnough(x, max)) return 1.f;
		else if (CloserEnough(x, 0.f)) return 0.f;
		return (FMath::LogX(10,(x / max) / (1.0f - (x / max))) + 5.0f) / 10.0f;
	}

	UE_NODISCARD static FORCEINLINE float ExponentielNeg(int x, float max, float exponentiel) {
		if (x <= 0.f) return 1.f;
		return FMath::Pow(exponentiel, -x / FMath::CeilToFloat(max / 5.));
	}

	UE_NODISCARD static FORCEINLINE float PolynomeDegre2(float a, float b, float c, float index) {
		return  FMath::Pow(index, 2) * a + b * index + c;
	}

};
