// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelGenerator.h"

#if WITH_EDITOR
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "Runtime/CoreUObject/Public/Templates/Casts.h"

#include "Editor/UnrealEd/Public/Editor.h"
#include "Editor/UnrealEd/Public/EditorModeManager.h"
#endif // WITH_EDITOR

ALevelGenerator::ALevelGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
#if WITH_EDITOR
	LevelTexture2D = nullptr;
#endif // WITH_EDITOR
}

#if WITH_EDITOR
void ALevelGenerator::GenerateLevel()
{
	if (LevelTexture2D != nullptr)
	{
		FTexture2DMipMap* LevelTextureMipMap = &LevelTexture2D->PlatformData->Mips[0];

		FByteBulkData* RawLevelTextureImageData = &LevelTextureMipMap->BulkData;

		const FColor* RawImageDataAsColor = static_cast<const FColor*>(RawLevelTextureImageData->Lock(LOCK_READ_ONLY));

		const int32 LevelTextureWidth = LevelTextureMipMap->SizeX;
		const int32 LevelTextureHeight = LevelTextureMipMap->SizeY;

		for (int32 Pixel_X = 0; Pixel_X < LevelTextureWidth; ++Pixel_X)
		{
			for (int32 Pixel_Y = 0; Pixel_Y < LevelTextureHeight; ++Pixel_Y)
			{
				const FColor& PixelColor = RawImageDataAsColor[Pixel_Y * LevelTextureWidth + Pixel_X];

				GenerateTile(Pixel_X, Pixel_Y, PixelColor);
			}
		}

		RawLevelTextureImageData->Unlock();

		GenerateLevelPlane(LevelTextureWidth, LevelTextureHeight);

		GEditor->EditorUpdateComponents();
		UWorld* CurrentWorld = GEditor->GetEditorWorldContext().World();
		CurrentWorld->UpdateWorldComponents(true, true);
		FEditorModeTools& EditorModeTools = GLevelEditorModeTools();
		EditorModeTools.MapChangeNotify();
	}
}

void ALevelGenerator::GenerateTile(int32 InPixel_X, int32 InPixel_Y, const FColor& InPixelColor)
{
	if (InPixelColor.A == 0)
	{
		return;
	}

	AActor** ActorToGenerate = ActorColorMappings.Find(InPixelColor);
	UStaticMesh** StaticMeshToGenerate = StaticMeshColorMappings.Find(InPixelColor);
	if (ActorToGenerate != nullptr || StaticMeshToGenerate != nullptr)
	{
		float* HeightOffset = HeightOffsetColorMappings.Find(InPixelColor);
		float* HeightScale = HeightScaleColorMappings.Find(InPixelColor);

		const float WorldScalePosition_X = float(InPixel_X) * WorldUnitScale + WorldRootOffset;
		const float WorldScalePosition_Y = float(InPixel_Y) * WorldUnitScale + WorldRootOffset;
		const float WorldScalePosition_Z = HeightOffset ? *HeightOffset : 0.f;
		const FVector NewActorPosition(WorldScalePosition_X, WorldScalePosition_Y, WorldScalePosition_Z);
		const FRotator NewActorRotationInDegrees(0.f, 0.f, 0.f);
		const FVector NewActorScale(1.f, 1.f, HeightScale ? *HeightScale : 1.f);

		ULevel* CurrentLevel = GEditor->GetEditorWorldContext().World()->GetCurrentLevel();
		UClass* ClassToGenerate = StaticMeshToGenerate ? AStaticMeshActor::StaticClass() : (*ActorToGenerate)->GetClass();
		const FTransform NewActorTransform(NewActorRotationInDegrees, NewActorPosition, NewActorScale);
		constexpr bool bLogAddActor = false;
		const EObjectFlags ObjectFlagsParam = RF_Public | RF_Transactional;
		AActor* NewActor = GEditor->AddActor(CurrentLevel, ClassToGenerate, NewActorTransform, bLogAddActor, ObjectFlagsParam);

		if (NewActor == nullptr)
		{
			return;
		}

		if (StaticMeshToGenerate)
		{
			AStaticMeshActor* NewStaticMeshActor = Cast<AStaticMeshActor>(NewActor);
			NewStaticMeshActor->GetStaticMeshComponent()->SetStaticMesh(*StaticMeshToGenerate);
			NewStaticMeshActor->SetActorScale3D(NewActorScale);
		}

		const FString NewActorName = FString::Printf(TEXT("%s_%f_%f_%f"), *(ClassToGenerate->GetName()), NewActorPosition.X, NewActorPosition.Y, NewActorPosition.Z);

		NewActor->Rename(*NewActorName);
		NewActor->SetActorLabel(NewActorName);

		NewActor->ReregisterAllComponents();
		NewActor->RerunConstructionScripts();
	}
}

void ALevelGenerator::GenerateLevelPlane(int32 InLevelTextureWidth, int32 InLevelTextureHeight)
{
	const FRotator LevelPlaneRotation(0.f, 0.f, 0.f);
	const FVector LevelPlanePosition(float(InLevelTextureWidth) * WorldUnitScale / 2.f, float(InLevelTextureHeight) * WorldUnitScale / 2.f, 0.f);
	const FVector LevelPlaneScale(InLevelTextureWidth, InLevelTextureHeight, 1.f);
	const FTransform LevelPlaneTransform(LevelPlaneRotation, LevelPlanePosition, LevelPlaneScale);

	AActor* LevelPlaneActor = GEditor->AddActor(GEditor->GetEditorWorldContext().World()->GetCurrentLevel()
												, AStaticMeshActor::StaticClass()
												, LevelPlaneTransform
												, true
												, RF_Public | RF_Transactional);

	AStaticMeshActor* LevelPlaneStaticMeshActor = Cast<AStaticMeshActor>(LevelPlaneActor);
	LevelPlaneStaticMeshActor->GetStaticMeshComponent()->SetStaticMesh(Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), this, TEXT("/Engine/BasicShapes/Plane"))));
	LevelPlaneStaticMeshActor->SetActorScale3D(LevelPlaneScale);

	const FString LevelPlaneActorName = TEXT("StaticMesh_Plane");
	LevelPlaneActor->Rename(*LevelPlaneActorName);
	LevelPlaneActor->SetActorLabel(LevelPlaneActorName);

	LevelPlaneActor->ReregisterAllComponents();
	LevelPlaneActor->RerunConstructionScripts();
}

#endif // WITH_EDITOR
