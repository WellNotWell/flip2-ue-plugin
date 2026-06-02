#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TextureResource.h"

THIRD_PARTY_INCLUDES_START
#include "image.h"
THIRD_PARTY_INCLUDES_END

/**
 * Convert a UTexture2D to the FLIP Image format.
 * Reads pixel data from the first mip level.
 */
static Image TextureToFlipImage(UTexture2D* Texture)
{
	check(Texture);

	Image Img;
	Img.width = Texture->GetSizeX();
	Img.height = Texture->GetSizeY();
	Img.pixels.resize(Img.width * Img.height * 4);

#if ENGINE_MAJOR_VERSION >= 5
	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
#else
	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
#endif

	const void* RawData = Mip.BulkData.LockReadOnly();

	if (!RawData)
	{
		Mip.BulkData.Unlock();
		UE_LOG(LogTemp, Error, TEXT("FlipV2: Failed to lock texture mip data for '%s'"), *Texture->GetName());
		return Img;
	}

	const FColor* Colors = static_cast<const FColor*>(RawData);
	const int32 NumPixels = Img.width * Img.height;

	for (int32 i = 0; i < NumPixels; ++i)
	{
		// UE stores as BGRA, convert to RGBA float [0,1]
		Img.pixels[i * 4 + 0] = Colors[i].R / 255.0f;
		Img.pixels[i * 4 + 1] = Colors[i].G / 255.0f;
		Img.pixels[i * 4 + 2] = Colors[i].B / 255.0f;
		Img.pixels[i * 4 + 3] = Colors[i].A / 255.0f;
	}

	Mip.BulkData.Unlock();
	return Img;
}

/**
 * Convert a UTextureRenderTarget2D to the FLIP Image format.
 * Reads pixels from the render target resource on the game thread.
 */
static Image RenderTargetToFlipImage(UTextureRenderTarget2D* RT)
{
	check(RT);

	Image Img;
	Img.width = RT->SizeX;
	Img.height = RT->SizeY;
	Img.pixels.resize(Img.width * Img.height * 4);

	FTextureRenderTargetResource* RTResource = RT->GameThread_GetRenderTargetResource();
	if (!RTResource)
	{
		UE_LOG(LogTemp, Error, TEXT("FlipV2: Failed to get render target resource for '%s'"), *RT->GetName());
		return Img;
	}

	TArray<FColor> Pixels;
	RTResource->ReadPixels(Pixels);

	const int32 NumPixels = Img.width * Img.height;
	for (int32 i = 0; i < NumPixels; ++i)
	{
		Img.pixels[i * 4 + 0] = Pixels[i].R / 255.0f;
		Img.pixels[i * 4 + 1] = Pixels[i].G / 255.0f;
		Img.pixels[i * 4 + 2] = Pixels[i].B / 255.0f;
		Img.pixels[i * 4 + 3] = Pixels[i].A / 255.0f;
	}

	return Img;
}

/**
 * Convert a FLIP Image (Jet-colored heatmap) to a transient UTexture2D.
 * The caller is responsible for keeping the texture alive.
 */
static UTexture2D* FlipImageToTexture(const Image& Img)
{
	if (Img.empty()) return nullptr;

	UTexture2D* Texture = UTexture2D::CreateTransient(Img.width, Img.height, PF_B8G8R8A8);
	if (!Texture) return nullptr;

	Texture->SRGB = true;
	Texture->Filter = TF_Bilinear;

#if ENGINE_MAJOR_VERSION >= 5
	uint8* MipData = static_cast<uint8*>(Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
#else
	uint8* MipData = static_cast<uint8*>(Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
#endif

	const int32 NumPixels = Img.width * Img.height;
	for (int32 i = 0; i < NumPixels; ++i)
	{
		// FLIP Image is RGBA float, UE expects BGRA uint8
		MipData[i * 4 + 0] = FMath::Clamp(static_cast<int32>(Img.pixels[i * 4 + 2] * 255.0f), 0, 255); // B
		MipData[i * 4 + 1] = FMath::Clamp(static_cast<int32>(Img.pixels[i * 4 + 1] * 255.0f), 0, 255); // G
		MipData[i * 4 + 2] = FMath::Clamp(static_cast<int32>(Img.pixels[i * 4 + 0] * 255.0f), 0, 255); // R
		MipData[i * 4 + 3] = 255; // A
	}

#if ENGINE_MAJOR_VERSION >= 5
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
#else
	Texture->PlatformData->Mips[0].BulkData.Unlock();
#endif

	Texture->UpdateResource();
	return Texture;
}
