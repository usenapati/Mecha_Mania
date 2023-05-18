// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSHUD.h"

void AFPSHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		FVector2D Spread(0.f, 0.f);
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		if (HUDPackage.CrosshairCenter)
		{
			DrawCrossHair(HUDPackage.CrosshairCenter, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairLeft)
		{
			Spread = {-SpreadScaled, 0};
			DrawCrossHair(HUDPackage.CrosshairLeft, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairRight)
		{
			Spread = {SpreadScaled, 0};
			DrawCrossHair(HUDPackage.CrosshairRight, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairTop)
		{
			Spread = {0, -SpreadScaled};
			DrawCrossHair(HUDPackage.CrosshairTop, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairBottom)
		{
			Spread = {0, SpreadScaled};
			DrawCrossHair(HUDPackage.CrosshairBottom, ViewportCenter, Spread);
		}
	}
}

void AFPSHUD::DrawCrossHair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		FLinearColor::White
	);
}
