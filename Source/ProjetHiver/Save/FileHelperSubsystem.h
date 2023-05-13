#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "JsonObjectConverter.h"
#include "FileHelperSubsystem.generated.h"

/**
 * File Helper Subsytem
 */
UCLASS()
class PROJETHIVER_API UFileHelperSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Serialize a struct into a string with the json format.
	 * Template method, struct of object in parameter.
	 * 
	 * @param Struct, object to serialize
	 * @return the string containing the object serialized with json format
	 */
	template<class T>
	FString JsonSerializeStruct(const T& Struct) const
	{
		TSharedRef<FJsonObject> JsonStatham = MakeShareable(new FJsonObject());
		FJsonObjectConverter::UStructToJsonObject(T::StaticStruct(), &Struct, JsonStatham, 0, 0);

		FString JsonStr;
		const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonStr);
		FJsonSerializer::Serialize(JsonStatham, JsonWriter);

		return JsonStr;
	}

	/**
	 * Deserialize a string with the json format into a struct.
	 * Template method, the struct of returned object.
	 *
	 * @param JsonStr, string to deserialize
	 * @return the struct obtained from the string
	 */
	template<class T>
	T JsonDeserializeStruct(const FString& JsonStr) const
	{
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonStr);
		TSharedPtr<FJsonObject> JsonStatham;
		FJsonSerializer::Deserialize(JsonReader, JsonStatham);

		T Struct;
		FJsonObjectConverter::JsonObjectToUStruct(
			JsonStatham.ToSharedRef(), T::StaticStruct(), &Struct);

		return Struct;
	}

	/**
	 * Deserialize a string with the json format into a JsonObject.
	 *
	 * @param JsonStr, string to deserialize
	 * @return the JsonObject obtained from the string
	 */
	TSharedPtr<FJsonObject> JsonDeserialize(const FString& JsonStr) const
	{
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonStr);
		TSharedPtr<FJsonObject> JsonStatham;
		FJsonSerializer::Deserialize(JsonReader, JsonStatham);

		return JsonStatham;
	}

	/**
	 * Deserialize a string with the json format into an array of JsonValue.
	 *
	 * @param JsonStr, string to deserialize
	 * @return the array of JsonValue obtained from the string
	 */
	TArray<TSharedPtr<FJsonValue>> JsonDeserializeArray(const FString& JsonStr) const
	{
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonStr);
		TArray<TSharedPtr<FJsonValue>> JsonStatham;
		FJsonSerializer::Deserialize(JsonReader, JsonStatham);

		return JsonStatham;
	}

	/**
	 * Write a string into a file.
	 * If the file exists, it will be overrided.
	 * 
	 * @param Path, the path where the file is
	 * @param Str, the string to write
	 */
	void WriteFile(const FString& Path, const FString& Str) const;

	/**
	 * Read a file content into a string.
	 * If the file doesn't exist, return an empty string.
	 * 
	 * @param Path, the path of the file to read
	 * @return the file content in a string
	 */
	FString ReadFile(const FString& Path) const;

	/**
	 * Find all json files with a specific name into a directory.
	 * 
	 * @param Directory, the directory in which to search files
	 * @param FileName, the name of searched files
	 * @return the array containing the path of each file found
	 */
	TArray<FString> FindFiles(const FString& Directory, const FString& FileName) const;

	/**
	 * Delete a directory from disk
	 * 
	 * @param Directory, the directory to delete
	 */
	void DeleteDirectory(const FString& Directory) const;
};
