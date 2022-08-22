// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SLTilemapLib.generated.h"



/*
UENUM(BlueprintType, Meta = (Bitflags))
enum class ETileState : uint8
{
	Void,
	Ground,
	Wall,
	Window,
	RoofedVoid,
	RoofedGround,
	RoofedWall,
	RoofedWindow
};
*/


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
//struct SLTILEMAP_API FTileMap : public FTableRowBase
struct FTileMap : public FTableRowBase
{
	GENERATED_BODY()
	FTileMap()
	{
		Width = 3;
		Height = 3;
		Data.Init(0, Width*Height);
	}

	FTileMap(const int32 NewWidth, const int32 NewHeight)
	{
		Width = NewWidth;
		Height = NewHeight;
		Data.Init(0, Width*Height);
	}

	FTileMap(const int32 NewWidth, const int32 NewHeight, const uint8 InitialValue)
	{
		Width = NewWidth;
		Height = NewHeight;
		Data.Init(InitialValue, Width*Height);
	}

	FTileMap(const int32 NewWidth, const int32 NewHeight, const TArray<uint8> NewData)
	{
		Width = NewWidth;
		Height = NewHeight;
		Data = NewData;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilemap")
	int32 Width;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilemap")
	int32 Height;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilemap", meta = (Bitmask, BitmaskEnum = "ETileState"))
	TArray<uint8> Data;
};

FORCEINLINE bool operator ==(const FTileMap& A, const FTileMap& B)
{
	//return true;
	return A.Width == B.Width && A.Data == B.Data;
}



USTRUCT(BlueprintType)
struct FPatternCell
{
	GENERATED_BODY()
	FPatternCell()
	{
	}

	FPatternCell(const int32 NewXPosition, const int32 NewYPosition, const TArray<int32> NewIndicesOfNeighbors,const TArray<bool> NewPatternIsAllowedArray)
	{
		X = NewXPosition;
		Y = NewYPosition;
		IndicesOfNeighbors = NewIndicesOfNeighbors;
		bAllowedPatterns = NewPatternIsAllowedArray;
		Entropy = bAllowedPatterns.Num();
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	int32 X = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	int32 Y = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	int32 Entropy = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	bool bIsObserved = false;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	//bool bVisited = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	TArray<int32> IndicesOfNeighbors;
	TArray<FPatternCell*> PointersToNeighbors;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	TArray<bool> bAllowedPatterns;
};


UCLASS()
class SLTILEMAP_API USLTilemapLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static FTileMap CreateTileMap(const int32 NewWidth, const int32 NewHeight, const uint8 InitialValue);
	
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static int32 TileMapIndexToX(const FTileMap& TileMap, const int32 Index);

	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static int32 TileMapIndexToY(const FTileMap& TileMap, const int32 Index);

	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static int32 TileMapXYToIndex(const FTileMap& TileMap, const int32 x, const int32 y);
		
	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static uint8 GetDataAtIndex(const FTileMap& TileMap, const int32 Index);

	UFUNCTION(BlueprintCallable, Category = "SLTileMap")
	static void SetTileAtXY(UPARAM(ref) FTileMap& TileMap, const uint8 Tile, const int32 x, const int32 y);

	UFUNCTION(BlueprintPure, Category = "SLTileMap")
	static uint8 GetTileAtXY(const FTileMap& TileMap, const int32 x, const int32 y);

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
	static FTileMap GetTilemapSection(const FTileMap& TileMap, const int32 x, const int32 y, const int32 SectionWidth, const int32 SectionHeight);
};
