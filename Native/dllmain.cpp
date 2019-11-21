// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

using Microsoft::WRL::ComPtr;

ComPtr<IWICImagingFactory> g_wicImagingFactory;
ComPtr<ID2D1Factory7> g_d2dFactory;
ComPtr<ID2D1DCRenderTarget> g_renderTarget;
ComPtr<ID2D1Bitmap> g_d2dSpritesheetBitmap;
std::vector<D2D1_RECT_F> g_spritesheetRects;
int g_spriteIndex;
int g_spriteWidth;
int g_spriteHeight;

struct SpritesheetBitmapData
{
	std::vector<UINT> DestBuffer;
	D2D1_SIZE_U Size;
};
SpritesheetBitmapData g_spritesheetBitmapData;

struct TimerInfo
{
	HWND ParentDialog;
	UINT_PTR EventId;
};
std::unique_ptr<TimerInfo> g_timer;
bool g_autoplay;

void VerifyHR(HRESULT hr)
{
	if (FAILED(hr))
		__debugbreak();
}

void VerifyBool(BOOL b)
{
	if (!b)
		__debugbreak();
}

extern "C" __declspec(dllexport) bool _stdcall Initialize(int clientWidth, int clientHeight, HDC hdc)
{
	D2D1_FACTORY_OPTIONS factoryOptions = {};
	factoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	VerifyHR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &factoryOptions, &g_d2dFactory));

	D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), 0, 0);

	g_d2dFactory->CreateDCRenderTarget(&renderTargetProperties, &g_renderTarget);

	RECT dcRect{};
	dcRect.right = clientWidth;
	dcRect.bottom = clientHeight;
	g_renderTarget->BindDC(hdc, &dcRect);

	g_spriteIndex = 0;
	g_autoplay = false;
	
	return true;
}

extern "C" __declspec(dllexport) void _stdcall OnResize(int clientWidth, int clientHeight, HDC hdc)
{
	RECT dcRect{};
	dcRect.right = clientWidth;
	dcRect.bottom = clientHeight;
	g_renderTarget->BindDC(hdc, &dcRect);
}

void UpdateSpritesheetRects()
{
	D2D1_SIZE_F spritesheetSize = g_d2dSpritesheetBitmap->GetSize();

	g_spritesheetRects.clear();

	UINT sheetWidth = static_cast<int>(spritesheetSize.width);
	UINT sheetHeight = static_cast<int>(spritesheetSize.height);

	for (UINT y = 0; y < sheetHeight; y += g_spriteHeight)
	{
		for (UINT x = 0; x < sheetWidth; x += g_spriteWidth)
		{
			D2D1_RECT_F rect;
			rect.left = x;
			rect.right = x + g_spriteWidth;
			rect.right = min(rect.right, sheetWidth);
			rect.top = y;
			rect.bottom = y + g_spriteHeight;
			rect.bottom = min(rect.bottom, sheetHeight);
			g_spritesheetRects.push_back(rect);
		}
	}
}

void EnsureWicImagingFactory()
{
	if (!g_wicImagingFactory)
	{
		if (FAILED(CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			(LPVOID*)& g_wicImagingFactory)))
		{
			return;
		}
	}
}

extern "C" __declspec(dllexport) void _stdcall OpenSpritesheetFile(HWND dialogParent)
{
	TCHAR documentsPath[MAX_PATH];

	VerifyHR(SHGetFolderPath(NULL,
		CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
		NULL,
		0,
		documentsPath));

	OPENFILENAME ofn;       // common dialog box structure
	wchar_t szFile[MAX_PATH];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = dialogParent;
	ofn.lpstrFile = szFile;

	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = L'\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"All\0*.*\0PNG Images\0*.PNG\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = documentsPath;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 

	if (GetOpenFileName(&ofn) == 0)
		return;

	EnsureWicImagingFactory();

	ComPtr<IWICBitmapDecoder> spDecoder;
	if (FAILED(g_wicImagingFactory->CreateDecoderFromFilename(
		ofn.lpstrFile,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad, &spDecoder)))
	{
		return;
	}

	ComPtr<IWICBitmapFrameDecode> spSource;
	if (FAILED(spDecoder->GetFrame(0, &spSource)))
	{
		return;
	}

	// Convert the image format to 32bppPBGRA, equiv to DXGI_FORMAT_B8G8R8A8_UNORM
	ComPtr<IWICFormatConverter> spConverter;
	if (FAILED(g_wicImagingFactory->CreateFormatConverter(&spConverter)))
	{
		return;
	}

	if (FAILED(spConverter->Initialize(
		spSource.Get(),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		NULL,
		0.f,
		WICBitmapPaletteTypeMedianCut)))
	{
		return;
	}

	if (FAILED(spConverter->GetSize(&g_spritesheetBitmapData.Size.width, &g_spritesheetBitmapData.Size.height)))
	{
		return;
	}

	g_spritesheetBitmapData.DestBuffer.resize(g_spritesheetBitmapData.Size.width * g_spritesheetBitmapData.Size.height);
	if (FAILED(spConverter->CopyPixels(
		NULL, 
		g_spritesheetBitmapData.Size.width * sizeof(UINT),
		g_spritesheetBitmapData.DestBuffer.capacity() * sizeof(UINT),
		(BYTE*)& g_spritesheetBitmapData.DestBuffer[0])))
	{
		return;
	}

	D2D1_BITMAP_PROPERTIES bitmapProperties = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
	if (FAILED(g_renderTarget->CreateBitmap(
		g_spritesheetBitmapData.Size, 
		&g_spritesheetBitmapData.DestBuffer[0],
		g_spritesheetBitmapData.Size.width * sizeof(UINT), 
		bitmapProperties, 
		&g_d2dSpritesheetBitmap)))
	{
		return;
	}

	g_spriteWidth = g_spritesheetBitmapData.Size.width;
	g_spriteHeight = g_spritesheetBitmapData.Size.height;

	UpdateSpritesheetRects();
}

extern "C" __declspec(dllexport) void _stdcall Paint()
{
	g_renderTarget->BeginDraw();
	g_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	if (g_d2dSpritesheetBitmap)
	{
		D2D1_SIZE_F spritesheetSize = g_d2dSpritesheetBitmap->GetSize();
		
		D2D1_RECT_F destRect = D2D1::RectF(0, 0, g_spriteWidth, g_spriteHeight);
		D2D1_RECT_F sourceRect = g_spritesheetRects[g_spriteIndex];
		g_renderTarget->DrawBitmap(g_d2dSpritesheetBitmap.Get(), destRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, sourceRect);
	}

	VerifyHR(g_renderTarget->EndDraw());	
}

extern "C" __declspec(dllexport) int _stdcall GetSpritesheetWidth()
{
	D2D1_SIZE_F spritesheetSize = g_d2dSpritesheetBitmap->GetSize();
	return static_cast<int>(spritesheetSize.width);
}

extern "C" __declspec(dllexport) int _stdcall GetSpritesheetHeight()
{
	D2D1_SIZE_F spritesheetSize = g_d2dSpritesheetBitmap->GetSize();
	return static_cast<int>(spritesheetSize.height);
}

extern "C" __declspec(dllexport) void _stdcall SetSpriteWidth(int w)
{
	g_spriteWidth = w;
	UpdateSpritesheetRects();
}

extern "C" __declspec(dllexport) void _stdcall SetSpriteHeight(int h)
{
	g_spriteHeight = h;
	UpdateSpritesheetRects();
}

extern "C" __declspec(dllexport) void _stdcall PreviousSprite()
{
	if (g_spriteIndex > 0)
	{
		g_spriteIndex--;
	}
	else
	{
		g_spriteIndex = g_spritesheetRects.size() - 1;
	}
}

extern "C" __declspec(dllexport) void _stdcall NextSprite()
{
	if (g_spriteIndex < g_spritesheetRects.size() - 1)
	{
		++g_spriteIndex;
	}
	else
	{
		g_spriteIndex = 0;
	}
}

void EnsureTimerStopped()
{
	if (!g_timer)
		return;

	KillTimer(g_timer->ParentDialog, g_timer->EventId);
	g_timer.reset();
}

void CALLBACK TimerProc(
	HWND hwnd,
	UINT message,
	UINT idTimer,
	DWORD dwTime)
{
	if (g_autoplay)
	{
		NextSprite();
		Paint();
	}
}

void StartTimer(HWND parent, UINT_PTR id, UINT speed)
{
	g_timer = std::make_unique<TimerInfo>();
	g_timer->ParentDialog = parent;
	g_timer->EventId = id;
	SetTimer(g_timer->ParentDialog, g_timer->EventId, speed, TimerProc);
}

extern "C" __declspec(dllexport) void _stdcall SetAutoplay(HWND parent, int value)
{
	bool newValue = value != 0;
	if (g_autoplay == newValue)
		return;

	g_autoplay = newValue;

	if (g_autoplay)
	{
		EnsureTimerStopped();
		StartTimer(parent, 1, 100);
	}
	else
	{
		EnsureTimerStopped();
	}
}

extern "C" __declspec(dllexport) void _stdcall SetAutoplaySpeed(HWND parent, int value)
{
	EnsureTimerStopped();
	StartTimer(parent, 1, value);
}

extern "C" __declspec(dllexport) void _stdcall SaveGif(int animationSpeed)
{
	EnsureWicImagingFactory();

	std::wstring destFilename = L"C:\\Users\\Claire\\source\\repos\\Spritesheet2Gif\\Debug\\export.gif";

	ComPtr<IWICStream> stream;
	if (FAILED(g_wicImagingFactory->CreateStream(&stream)))
	{
		return;
	}
	if (FAILED(stream->InitializeFromFilename(destFilename.c_str(), GENERIC_WRITE)))
	{
		return;
	}

	ComPtr<IWICBitmapEncoder> encoder;
	if (FAILED(g_wicImagingFactory->CreateEncoder(GUID_ContainerFormatGif, NULL, &encoder)))
	{
		return;
	}

	if (FAILED(encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache)))
	{
		return;
	}

	ComPtr<IWICMetadataQueryWriter> globalMetadataQueryWriter;
	if (FAILED(encoder->GetMetadataQueryWriter(&globalMetadataQueryWriter)))
	{
		return;
	}

	{
		PROPVARIANT value;
		PropVariantInit(&value);
		
		unsigned char extSize = 3;
		unsigned char loopTypeIsAnimatedGif = 1;
		unsigned char loopCountLsb = 0;
		unsigned char loopCountMsb = 0;
		unsigned char data[] = { extSize, loopTypeIsAnimatedGif, loopCountLsb, loopCountMsb };

		value.vt = VT_UI1 | VT_VECTOR;
		value.caub.cElems = 4;
		value.caub.pElems = data;

		if (FAILED(globalMetadataQueryWriter->SetMetadataByName(L"/appext/data", &value)))
		{
			return;
		}
	}
	{
		PROPVARIANT val;
		PropVariantInit(&val);

		unsigned char data[] = "NETSCAPE2.0";

		val.vt = VT_UI1 | VT_VECTOR;
		val.caub.cElems = 11;
		val.caub.pElems = data;

		if (FAILED(globalMetadataQueryWriter->SetMetadataByName(L"/appext/application", &val)))
		{	
			return;
		}
	}

	ComPtr<IWICBitmap> singleSpriteWicBitmap;
	if (FAILED(g_wicImagingFactory->CreateBitmap(
		g_spritesheetBitmapData.Size.width,
		g_spritesheetBitmapData.Size.height,
		GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnDemand, &singleSpriteWicBitmap)))
	{
		return;
	}

	ComPtr<ID2D1RenderTarget> wicBitmapRenderTarget;
	D2D1_RENDER_TARGET_PROPERTIES d2dRenderTargetProperties =
		D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

	if (FAILED(g_d2dFactory->CreateWicBitmapRenderTarget(singleSpriteWicBitmap.Get(), d2dRenderTargetProperties, &wicBitmapRenderTarget)))
	{
		return;
	}

	// Create a local copy of the spritesheet which is WIC target compatible.
	ComPtr<ID2D1Bitmap> wicCompatibleSpritesheet;
	D2D1_BITMAP_PROPERTIES bitmapProperties = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
	if (FAILED(wicBitmapRenderTarget->CreateBitmap(
		g_spritesheetBitmapData.Size,
		&g_spritesheetBitmapData.DestBuffer[0],
		g_spritesheetBitmapData.Size.width * sizeof(UINT),
		bitmapProperties,
		&wicCompatibleSpritesheet)))
	{
		return;
	}

	int frameCount = g_spritesheetRects.size();

	for (int i = 0; i < frameCount; ++i)
	{
		ComPtr<IPropertyBag2> wicPropertyBag;
		ComPtr<IWICBitmapFrameEncode> frameEncode;
		if (FAILED(encoder->CreateNewFrame(&frameEncode, &wicPropertyBag)))
		{
			return;
		}

		if (FAILED(frameEncode->Initialize(wicPropertyBag.Get())))
		{
			return;
		}

		// Reference: https://docs.microsoft.com/en-us/windows/win32/wic/-wic-native-image-format-metadata-queries#gif-metadata
		// Cached MSDN docpage: https://webcache.googleusercontent.com/search?q=cache%3A7nMWvmIamxMJ%3Ahttps%3A%2F%2Fcode.msdn.microsoft.com%2FWindows-Imaging-Component-65abbc6a%2Fsourcecode%3FfileId%3D127204%26pathId%3D969071766&hl=en&gl=us&strip=1&vwsrc=0&fbclid=IwAR2ZTDktvIs0t-8fv_x33Y1t4SqgeMMR8TysjwWYy_JwjE49V31Yi3Sf6Kk
		
		ComPtr<IWICMetadataQueryWriter> frameMetadataQueryWriter;
		if (FAILED(frameEncode->GetMetadataQueryWriter(&frameMetadataQueryWriter)))
		{
			return;
		}

		{
			PROPVARIANT value;
			PropVariantInit(&value);
			value.vt = VT_UI2;
			value.uiVal = animationSpeed / 10;
			if (FAILED(frameMetadataQueryWriter->SetMetadataByName(L"/grctlext/Delay", &value)))
			{
				return;
			}
		}

		if (FAILED(frameEncode->SetSize(g_spriteWidth, g_spriteHeight)))
		{
			return;
		}

		if (FAILED(frameEncode->SetResolution(96, 96)))
		{
			return;
		}

		WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat32bppPBGRA;
		if (FAILED(frameEncode->SetPixelFormat(&pixelFormat)))
		{
			return;
		}

		wicBitmapRenderTarget->BeginDraw();

		wicBitmapRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::CornflowerBlue));

		D2D1_RECT_F destRect = D2D1::RectF(0, 0, g_spriteWidth, g_spriteHeight);
		D2D1_RECT_F sourceRect = g_spritesheetRects[i];
		wicBitmapRenderTarget->DrawBitmap(
			wicCompatibleSpritesheet.Get(), 
			destRect, 
			1.0f, 
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, 
			sourceRect);

		if (FAILED(wicBitmapRenderTarget->EndDraw()))
		{
			return;
		}

		if (FAILED(frameEncode->WriteSource(
			singleSpriteWicBitmap.Get(),
			NULL)))
		{
			return;
		}

		if (FAILED(frameEncode->Commit()))
		{
			return;
		}
	}

	if (FAILED(encoder->Commit()))
	{
		return;
	}

	if (FAILED(stream->Commit(STGC_DEFAULT)))
	{
		return;
	}
}

extern "C" __declspec(dllexport) void _stdcall Uninitialize()
{
	g_d2dFactory.Reset();
	g_renderTarget.Reset();
	g_d2dSpritesheetBitmap.Reset();
	g_wicImagingFactory.Reset();
}