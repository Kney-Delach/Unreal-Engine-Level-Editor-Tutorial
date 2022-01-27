// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Runtime/Core/Public/CoreMinimal.h"

#if WITH_EDITOR
#include "Runtime/Core/Public/Math/Color.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#endif // WITH_EDITOR

#include "LevelGenerator.generated.h"

#if WITH_EDITOR
class UStaticMesh;
#endif // WITH_EDITOR

UCLASS()
class THIRDPERSONSHOOTER_API ALevelGenerator : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevelGenerator();

#if WITH_EDITOR

	// Generates level content for the level active in the editor.
	UFUNCTION(CallInEditor, Category = "Level Generator")
	void GenerateLevel();

private:
	void GenerateTile(int32 InPixel_X, int32 InPixel_Y, const FColor& InPixelColor);

	void GenerateLevelPlane(int32 InLevelTextureWidth, int32 InLevelTextureHeight);

public:
	UPROPERTY(EditAnywhere, Category = "Level Generator")
	UTexture2D* LevelTexture2D;

	UPROPERTY(EditAnywhere, Category = "Level Generator|Color Mappings")
	TMap<FColor, AActor*> ActorColorMappings;

	UPROPERTY(EditAnywhere, Category = "Level Generator|Color Mappings")
	TMap<FColor, UStaticMesh*> StaticMeshColorMappings;

	UPROPERTY(EditAnywhere, Category = "Level Generator|Color Mappings")
	TMap<FColor, UStaticMesh*> InstancedStaticMeshColorMappings;

	UPROPERTY(EditAnywhere, Category = "Level Generator|Color Mappings")
	TMap<FColor, float> HeightOffsetColorMappings;

	UPROPERTY(EditAnywhere, Category = "Level Generator|Color Mappings")
	TMap<FColor, float> HeightScaleColorMappings;

private:
	const float WorldUnitScale = 100.f;

	const float WorldRootOffset = 50.f;

	TMap<FColor, AActor*> SpawnedISMActorColorMap;

#endif // WITH_EDITOR

};
