#include "SLTilemapLib.h"


FTileMap USLTilemapLib::CreateTileMap(const int32 NewWidth, const int32 NewHeight, const uint8 InitialValue)
{
	return FTileMap(NewWidth, NewHeight, InitialValue);
}

int32 USLTilemapLib::TileMapIndexToX(const FTileMap& TileMap, const int32 Index)
{
	return Index % TileMap.Width;
}

int32 USLTilemapLib::TileMapIndexToY(const FTileMap& TileMap, const int32 Index)
{
	return Index / TileMap.Width;
}

int32 USLTilemapLib::TileMapXYToIndex(const FTileMap& TileMap, const int32 x, const int32 y)
{
	return (y * TileMap.Width) + x;
}

uint8 USLTilemapLib::GetDataAtIndex(const FTileMap& TileMap, const int32 Index)
{
	return TileMap.Data[Index];
}

void USLTilemapLib::SetTileAtXY(FTileMap& TileMap, const uint8 Tile, const int32 x, const int32 y)
{
	const int32 TileIndex = TileMapXYToIndex(TileMap, x, y);
	TileMap.Data[TileIndex] = Tile;
}

uint8 USLTilemapLib::GetTileAtXY(const FTileMap& TileMap, const int32 x, const int32 y)
{
	return TileMap.Data[TileMapXYToIndex(TileMap, x, y)];
}

UTexture2D* USLTilemapLib::TileMapToTexture(FTileMap& TileMap)
{
	TArray<FColor> Colors;
	const int32 Width = TileMap.Width;
	const int32 Height = TileMap.Height;
	const int32 PixelCount = Width * Height;
	for (auto& Tile:TileMap.Data)
	{
		Colors.Add(TileToColor(Tile));
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8, "");
	void* Data = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	
	FMemory::Memcpy(Data, Colors.GetData(), PixelCount * 4);
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
	Texture->Filter = TF_Nearest;
	Texture->UpdateResource();

	return Texture;
}

FColor USLTilemapLib::TileToColor(const uint8 Tile)
{
	const ETileState Flags = static_cast<ETileState>(Tile);
	const uint8 Void = EnumHasAnyFlags(Flags, ETileState::Void | ETileState::RoofedVoid);
	const uint8 Ground = EnumHasAnyFlags(Flags, ETileState::Ground | ETileState::RoofedGround);
	const uint8 Wall = EnumHasAnyFlags(Flags, ETileState::Wall | ETileState::RoofedWall);
	const uint8 Window = EnumHasAnyFlags(Flags, ETileState::Window | ETileState::RoofedWindow);
	const uint8 Roofed = EnumHasAnyFlags(Flags, ETileState::RoofedVoid | ETileState::RoofedGround | ETileState::RoofedWall | ETileState::RoofedWindow);
	const uint8 IsPow2 = (Tile & (Tile - 1)) == 0;

	/*
	const uint8 Void = (Tile & (static_cast<uint8>(ETileState::Void) | static_cast<uint8>(ETileState::RoofedVoid))) != 0;
	const uint8 Ground = (Tile & (static_cast<uint8>(ETileState::Ground) | static_cast<uint8>(ETileState::RoofedGround))) != 0;
	const uint8 Wall = (Tile & (static_cast<uint8>(ETileState::Wall) | static_cast<uint8>(ETileState::RoofedWall))) != 0;
	const uint8 Window = (Tile & (static_cast<uint8>(ETileState::Window) | static_cast<uint8>(ETileState::RoofedWindow))) != 0;
	const uint8 Void = (Tile & 17) != 0;
	const uint8 Ground = (Tile & 34) != 0;
	const uint8 Wall = (Tile & 68) != 0;
	const uint8 Window = (Tile & 136) != 0;
	*/

	const uint8 R = (6 * IsPow2 + 2) * (Wall * 16 + Window * 16);
	const uint8 G = (6 * IsPow2 + 2) * (Window * 16 + Ground * 16);
	const uint8 B = (6 * IsPow2 + 2) * (Void * 16 + Ground * 8 + Window * 8);
	const uint8 A = 255 * Roofed;

	return FColor(R,G,B,A);
}


FTileMap USLTilemapLib::MirrorTilemap(const FTileMap& TileMap)
{
	FTileMap OutTileMap = TileMap;
	for (int32 y = 0; y < TileMap.Height; y++)
	{
		for (int32 x = 0; x < TileMap.Width; x++)
		{
			SetTileAtXY(OutTileMap, GetTileAtXY(TileMap, TileMap.Width - 1 - x, y), x, y);
		}
	}
	return OutTileMap;
}

FTileMap USLTilemapLib::RotateTilemap(const FTileMap& TilemapToRotate)
{
	FTileMap OutTilemap = TilemapToRotate;
	OutTilemap.Height = TilemapToRotate.Width;
	OutTilemap.Width = TilemapToRotate.Height;

	for (int32 y = 0; y < OutTilemap.Height; y++)
	{
		for (int32 x = 0; x < OutTilemap.Width; x++)
		{
			SetTileAtXY(OutTilemap, GetTileAtXY(TilemapToRotate, y, TilemapToRotate.Height - 1 - x), x, y);
		}
	}
	return OutTilemap;
}

bool USLTilemapLib::IsTilemapValid(const FTileMap& InTileTilemap)
{
	return InTileTilemap.Data.Num() == InTileTilemap.Width * InTileTilemap.Height;
}

FTileMap USLTilemapLib::GetTilemapSection(const FTileMap& Tilemap, const int32 x,
                                          const int32 y, const int32 width, const int32 height)
{
	FTileMap OutSection = FTileMap(width, height);

	for (int32 j = 0; j < height; j++)
	{
		for (int32 i = 0; i < width; i++)
		{
			const int32 temp = GetTileAtXY(Tilemap, x + i, y + j);
			SetTileAtXY(OutSection, temp, i, j);
		}
	}
	return OutSection;
}
