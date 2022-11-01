#include "../tomb3/pch.h"
#include "texture.h"
#include "dd.h"

#define bSetColorKey	VAR_(0x004CEEC4, bool)
#define bMakeGrey	VAR_(0x006CED60, bool)

long DXTextureNewPalette(uchar* palette)
{
	ulong data[256];

	if (DXPalette)
	{
		DXPalette->Release();
		DXPalette = 0;
	}

	for (int i = 0; i < 256; i++, palette += 3)
		data[i] = RGB(palette[0], palette[1], palette[2]);

	return App.lpDD->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256, (LPPALETTEENTRY)data, &DXPalette, 0);
}

void DXResetPalette(DXTEXTURE* tex)
{
	DXPalette = 0;
	bSetColorKey = 1;
}

void DXTextureSetGreyScale(bool set)
{
	bMakeGrey = set;
}

LPDIRECT3DTEXTUREX DXTextureGetInterface(LPDIRECTDRAWSURFACEX surf)
{
	LPDIRECT3DTEXTUREX tex;

	if (SUCCEEDED(surf->QueryInterface(D3DTEXGUID, (LPVOID*)&tex)))
		return tex;
	
	return 0;
}

long DXTextureFindTextureSlot(DXTEXTURE* tex)
{
	for (int i = 0; i < 32; i++)
	{
		if (!(tex[i].dwFlags & 1))
			return i;
	}

	return -1;
}

bool DXTextureMakeSystemSurface(DXTEXTURE* tex, LPDDPIXELFORMAT ddpf)
{
	DDSURFACEDESCX desc;

	memset(&desc, 0, sizeof(DDSURFACEDESCX));
	desc.dwSize = sizeof(DDSURFACEDESCX);
	desc.dwHeight = tex->nHeight;
	desc.dwWidth = tex->nWidth;
	desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	desc.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY | DDSCAPS_TEXTURE;
	memcpy(&desc.ddpfPixelFormat, ddpf, sizeof(DDPIXELFORMAT));

	if (FAILED(DD_CreateSurface(desc, tex->pSystemSurface)))
		return 0;

	return !tex->pPalette || SUCCEEDED(tex->pSystemSurface->SetPalette(tex->pPalette));
}

long DXTextureMakeDeviceSurface(long w, long h, LPDIRECTDRAWPALETTE palette, DXTEXTURE* list)
{
	DXTEXTURE* tex;
	LPDDPIXELFORMAT ddpf;
	long index;

	index = DXTextureFindTextureSlot(list);

	if (index < 0)
		return -1;

	tex = &list[index];
	memset(tex, 0, sizeof(DXTEXTURE));
	tex->nWidth = w;
	tex->nHeight = h;
	tex->dwFlags = 1;
	tex->pPalette = palette;
	ddpf = &App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[App.DXConfigPtr->D3DTF].ddsd.ddpfPixelFormat;
	tex->bpp = ddpf->dwRGBBitCount;

	if (DXTextureMakeSystemSurface(tex, ddpf))
		return index;

	return -1;
}

void inject_texture(bool replace)
{
	INJECT(0x004B1B80, DXTextureNewPalette, replace);
	INJECT(0x004B1FB0, DXResetPalette, replace);
	INJECT(0x004B1B70, DXTextureSetGreyScale, replace);
	INJECT(0x004B1FD0, DXTextureGetInterface, replace);
	INJECT(0x004B2000, DXTextureFindTextureSlot, replace);
	INJECT(0x004B2020, DXTextureMakeSystemSurface, replace);
	INJECT(0x004B20C0, DXTextureMakeDeviceSurface, replace);
}
