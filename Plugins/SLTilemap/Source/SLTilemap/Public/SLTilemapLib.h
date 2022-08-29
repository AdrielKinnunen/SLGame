// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SLTilemapLib.generated.h"

UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ETileState : uint8
{
	None			= 0 UMETA(Hidden),
	Void			= 1<<0,
	Ground			= 1<<1,
	Wall			= 1<<2,
	Window			= 1<<3,
	RoofedVoid		= 1<<4,
	RoofedGround	= 1<<5,
	RoofedWall		= 1<<6,
	RoofedWindow	= 1<<7
};
ENUM_CLASS_FLAGS(ETileState);

USTRUCT(BlueprintType)
struct FTileMap : public FTableRowBase
{
	GENERATED_BODY()
	FTileMap()
	{
		SizeX = 3;
		SizeY = 3;
		Data.Init(0, SizeX*SizeY);
	}

	FTileMap(const int32 NewSizeX, const int32 NewSizeY)
	{
		SizeX = NewSizeX;
		SizeY = NewSizeY;
		Data.Init(0, SizeX*SizeY);
	}

	FTileMap(const int32 NewSizeX, const int32 NewSizeY, const uint8 InitialValue)
	{
		SizeX = NewSizeX;
		SizeY = NewSizeY;
		Data.Init(InitialValue, SizeX*SizeY);
	}

	FTileMap(const int32 NewSizeX, const int32 NewSizeY, const TArray<uint8> NewData)
	{
		SizeX = NewSizeX;
		SizeY = NewSizeY;
		Data = NewData;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilemap")
	int32 SizeX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilemap")
	int32 SizeY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilemap", meta = (Bitmask, BitmaskEnum = "ETileState"))
	TArray<uint8> Data;
};

FORCEINLINE bool operator ==(const FTileMap& A, const FTileMap& B)
{
	//return true;
	return A.SizeX == B.SizeX && A.Data == B.Data;
}


UCLASS()
class SLTILEMAP_API USLTilemapLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static FTileMap CreateTileMap(const int32 NewSizeX, const int32 NewSizeY, const uint8 InitialValue);
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static int32 TileMapIndexToX(const FTileMap& TileMap, const int32 Index);
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static int32 IndexToX(const int32 SizeX, const int32 Index);
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static int32 TileMapIndexToY(const FTileMap& TileMap, const int32 Index);
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static int32 IndexToY(const int32 SizeX, const int32 Index);
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static int32 TileMapXYToIndex(const FTileMap& TileMap, const int32 X, const int32 Y);
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static int32 XYToIndex(const int32 SizeX, const int32 X, const int32 Y);
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static uint8 GetDataAtIndex(const FTileMap& TileMap, const int32 Index);
	UFUNCTION(BlueprintCallable, Category = "SLTileMap")
	static void SetTileAtXY(UPARAM(ref) FTileMap& TileMap, const uint8 Tile, const int32 X, const int32 Y);
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static uint8 GetTileAtXY(const FTileMap& TileMap, const int32 X, const int32 Y);
	UFUNCTION(BlueprintCallable, Category = "SLTileMap")
	static UTexture2D* TileMapToTexture(UPARAM(ref) FTileMap& TileMap);
	UFUNCTION(BlueprintCallable, Category = "SLTileMap")
	static FColor TileToColor(const uint8 Tile);
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static FTileMap MirrorTilemap(const FTileMap& TileMap);
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static FTileMap RotateTilemap(const FTileMap& TileMap);
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static bool IsTilemapValid(const FTileMap& TileMap);
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static FTileMap GetTilemapSection(const FTileMap& TileMap, const int32 X, const int32 Y, const int32 SectionSizeX, const int32 SectionSizeY);
};
