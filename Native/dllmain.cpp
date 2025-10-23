// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

using Microsoft::WRL::ComPtr;

#include <limits>

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

struct GlobalAllocation
{
	void* m_locked;
	HGLOBAL m_handle;
	ComPtr<IStream> m_stream;

	GlobalAllocation() : m_handle(0), m_locked(0) {}

	~GlobalAllocation()
	{
		if (m_handle)
		{
			Uninitialize();
		}
	}

	void Initialize(size_t sizeInBytes)
	{
		m_handle = GlobalAlloc(0, sizeInBytes);
		m_locked = GlobalLock(m_handle);
	}

	void Uninitialize()
	{
		GlobalUnlock(m_handle);
		GlobalFree(m_handle);
		m_locked = 0;
		m_handle = 0;
	}
};

ComPtr<IWICImagingFactory> g_wicImagingFactory;
ComPtr<ID2D1Factory7> g_d2dFactory;
ComPtr<ID2D1DCRenderTarget> g_renderTarget;
std::wstring g_sourceFilePath;
int g_frameIndex;
bool g_loaded = false;
std::vector<ComPtr<ID2D1Bitmap>> g_colorizedFrames;

struct WindowTitleHelper
{
	std::wstring m_programName;
	std::wstring m_currentFile;
	std::wstring m_zoomFactor;
	std::wstring m_frameIndex;

	void UpdateWindowTitle(HWND dialogParent)
	{
		std::wstringstream strstrm;
		strstrm << m_programName.c_str();
		
		if (m_currentFile.size() > 0)
		{
			strstrm << L" - " << m_currentFile.c_str();
		}

		strstrm << L" - Frame " << m_frameIndex.c_str();

		strstrm << L" - " << m_zoomFactor.c_str();

		SetWindowText(dialogParent, strstrm.str().c_str());
	}

	void SetFrameNumberImpl(int frameNumber)
	{
		std::wstringstream strm;
		strm << frameNumber;
		m_frameIndex = strm.str();
	}

public:
	void Initialize(HWND dialogParent, wchar_t const* zoomFactorString, int frameNumber)
	{
		m_programName = L"Spritesheet2Gif";
		m_zoomFactor = zoomFactorString;
		SetFrameNumberImpl(frameNumber);
	}

	void SetZoomFactor(HWND dialogParent, wchar_t const* zoomFactorString)
	{
		m_zoomFactor = zoomFactorString;
		UpdateWindowTitle(dialogParent);
	}

	void SetFrameNumber(HWND dialogParent, int frameNumber)
	{
		if (dialogParent)
		{
			SetFrameNumberImpl(frameNumber);
			UpdateWindowTitle(dialogParent);
		}
	}

	void SetOpenFileName(HWND dialogParent, std::wstring fullPath)
	{
		size_t delimiterIndex = g_sourceFilePath.rfind(L'\\');
		assert(delimiterIndex < g_sourceFilePath.size());
		if (delimiterIndex >= g_sourceFilePath.size())
			return;

		m_currentFile = g_sourceFilePath.substr(delimiterIndex + 1);
		UpdateWindowTitle(dialogParent);
	}
};

WindowTitleHelper g_windowTitleHelper;

static const float g_zoomPercents[] = {6.75f, 12.5f, 25, 50, 100, 200, 300, 400, 500, 600, 700, 800, 1000, 1200, 1400, 1600};
static const wchar_t* g_zoomPercentStrings[] = {L"6.75f%", L"12.5f%", L"25%", L"50%", L"100%", L"200%", L"300%", L"400%", L"500%", L"600%", L"700%", L"800%", L"1000%", L"1200%", L"1400%", L"1600"};

static const int g_defaultZoomIndex = 4;
int g_zoomIndex;

struct PaletteEntry
{
	unsigned int R8G8B8;
};

struct Palette
{
	std::vector<PaletteEntry> PaletteEntries;
};

struct SpritesheetBitmapData
{
	std::vector<UINT> SourceBuffer;
	D2D1_SIZE_U SourceSize;

	D2D1_SIZE_U TargetSize;

	Palette Reference;
	std::vector<Palette> Palettes;
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

extern "C" __declspec(dllexport) bool _stdcall Initialize(int clientWidth, int clientHeight, HWND parentDialog, HDC hdc)
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

	g_frameIndex = 0;
	g_autoplay = false;
	g_zoomIndex = g_defaultZoomIndex;

	g_windowTitleHelper.Initialize(parentDialog, g_zoomPercentStrings[g_zoomIndex], g_frameIndex);
	
	return true;
}

extern "C" __declspec(dllexport) void _stdcall OnResize(int clientWidth, int clientHeight, HDC hdc)
{
	RECT dcRect{};
	dcRect.right = clientWidth;
	dcRect.bottom = clientHeight;
	g_renderTarget->BindDC(hdc, &dcRect);
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

bool TryLoadAsRaster(std::wstring fileName)
{
	ComPtr<IWICBitmapDecoder> spDecoder;
	if (FAILED(g_wicImagingFactory->CreateDecoderFromFilename(
		g_sourceFilePath.c_str(),
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad, &spDecoder)))
	{
		return false;
	}

	ComPtr<IWICBitmapFrameDecode> spSource;
	if (FAILED(spDecoder->GetFrame(0, &spSource)))
	{
		return false;
	}

	// Convert the image format to 32bppPBGRA, equiv to DXGI_FORMAT_B8G8R8A8_UNORM
	ComPtr<IWICFormatConverter> spConverter;
	if (FAILED(g_wicImagingFactory->CreateFormatConverter(&spConverter)))
	{
		return false;
	}

	if (FAILED(spConverter->Initialize(
		spSource.Get(),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		NULL,
		0.f,
		WICBitmapPaletteTypeMedianCut)))
	{
		return false;
	}

	if (FAILED(spConverter->GetSize(&g_spritesheetBitmapData.SourceSize.width, &g_spritesheetBitmapData.SourceSize.height)))
	{
		return false;
	}

	// Copy pixels into a vector
	g_spritesheetBitmapData.SourceBuffer.resize(g_spritesheetBitmapData.SourceSize.width * g_spritesheetBitmapData.SourceSize.height);
	if (FAILED(spConverter->CopyPixels(
		NULL,
		g_spritesheetBitmapData.SourceSize.width * sizeof(UINT),
		static_cast<UINT>(g_spritesheetBitmapData.SourceBuffer.size()) * sizeof(UINT),
		(BYTE*)&g_spritesheetBitmapData.SourceBuffer[0])))
	{
		return false;
	}

	// Copy the reference
	g_spritesheetBitmapData.Reference.PaletteEntries.resize(8);
	for (int colorIndex = 0; colorIndex < 8; ++colorIndex)
	{
		int sourceX = colorIndex;
		g_spritesheetBitmapData.Reference.PaletteEntries[colorIndex].R8G8B8 = g_spritesheetBitmapData.SourceBuffer[sourceX];
	}

	// Copy the palettes
	g_spritesheetBitmapData.Palettes.resize(7);
	for (int frameIndex = 0; frameIndex < 7; ++frameIndex)
	{
		// For each palette
		g_spritesheetBitmapData.Palettes[frameIndex].PaletteEntries.resize(8);
		for (int colorIndex = 0; colorIndex < 8; ++colorIndex)
		{
			// For each color
			int sourceX = (frameIndex + 1) * 8 + colorIndex;
			g_spritesheetBitmapData.Palettes[frameIndex].PaletteEntries[colorIndex].R8G8B8 = g_spritesheetBitmapData.SourceBuffer[sourceX];
		}
	}

	g_colorizedFrames.resize(8);

	std::vector<UINT> colorizedBuffer;
	colorizedBuffer.resize(64 * 64);

	for (int frameIndex = 0; frameIndex < 7; ++frameIndex)
	{
		for (int y = 0; y < 64; ++y)
		{
			for (int x = 0; x < 64; ++x)
			{
				int sourceIndex = (y + 1) * 64 + x;
				UINT sourceColor = g_spritesheetBitmapData.SourceBuffer[sourceIndex];

				// What color index is it? Use the reference
				UINT sourceColorIndex = -1;
				for (int colorIndex = 0; colorIndex < 8; ++colorIndex)
				{
					if (sourceColor == g_spritesheetBitmapData.Reference.PaletteEntries[colorIndex].R8G8B8)
					{
						sourceColorIndex = colorIndex;
					}
				}							

				int destIndex = y * 64 + x;
				UINT destColor = 0xFFFF00FF; // Debug magenta
				if (sourceColorIndex != -1)
				{
					destColor = g_spritesheetBitmapData.Palettes[frameIndex].PaletteEntries[sourceColorIndex].R8G8B8;
				}
				colorizedBuffer[destIndex] = destColor;
			}
		}

		D2D1_SIZE_U frameSize = D2D1::SizeU(64, 64);

		D2D1_BITMAP_PROPERTIES bitmapProperties = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
		if (FAILED(g_renderTarget->CreateBitmap(
			frameSize,
			colorizedBuffer.data(),
			g_spritesheetBitmapData.SourceSize.width * sizeof(UINT),
			bitmapProperties,
			&g_colorizedFrames[frameIndex])))
		{
			return false;
		}
	}

	g_loaded = true;

	return true;
}

void LoadSpritesheetFileImpl()
{
	EnsureWicImagingFactory();

	if (!TryLoadAsRaster(g_sourceFilePath.c_str()))
	{
		assert(false);
	}

	return;

}

void OpenSpritesheetFileImpl(HWND dialogParent, std::wstring fileName)
{
	g_sourceFilePath = fileName;

	LoadSpritesheetFileImpl();

	g_windowTitleHelper.SetOpenFileName(dialogParent, g_sourceFilePath);

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

	OpenSpritesheetFileImpl(dialogParent, ofn.lpstrFile);
}

#if _DEBUG
extern "C" __declspec(dllexport) void _stdcall AutoOpenSpritesheetFile(HWND dialogParent)
{
	std::wstring testFilename = L"C:\\Users\\cmlan\\OneDrive\\Pictures\\rotation.png";

	OpenSpritesheetFileImpl(dialogParent, testFilename);
}
#endif

extern "C" __declspec(dllexport) void _stdcall Paint()
{
	g_renderTarget->BeginDraw();
	g_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	float zoomFactor = g_zoomPercents[g_zoomIndex] / 100.0f;

	if (g_loaded)
	{
		D2D1_MATRIX_3X2_F transform = D2D1::Matrix3x2F::Scale(zoomFactor, zoomFactor);
		g_renderTarget->SetTransform(transform);

		D2D1_RECT_F sourceRect = D2D1::RectF(0, 0, static_cast<float>(64), static_cast<float>(64));
		D2D1_RECT_F destRect = D2D1::RectF(0, 0, static_cast<float>(64), static_cast<float>(64));
		g_renderTarget->DrawBitmap(
			g_colorizedFrames[g_frameIndex].Get(),
			destRect, 
			1.0f, 
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, 
			sourceRect);
	}

	VerifyHR(g_renderTarget->EndDraw());	
}

extern "C" __declspec(dllexport) int _stdcall GetTargetWidth()
{
	return 64;
}

extern "C" __declspec(dllexport) int _stdcall GetTargetHeight()
{
	return 64;
}

extern "C" __declspec(dllexport) void _stdcall PreviousFrame(HWND parentDialog)
{
	if (g_frameIndex > 0)
	{
		g_frameIndex--;
	}
	else
	{
		g_frameIndex = 6;
	}
	g_windowTitleHelper.SetFrameNumber(parentDialog, g_frameIndex);
}

extern "C" __declspec(dllexport) void _stdcall NextFrame(HWND parentDialog)
{
	if (g_frameIndex < 6)
	{
		++g_frameIndex;
	}
	else
	{
		g_frameIndex = 0;
	}
	g_windowTitleHelper.SetFrameNumber(parentDialog, g_frameIndex);
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
    UINT_PTR idTimer,
	DWORD dwTime)
{
	if (g_autoplay)
	{
		NextFrame(nullptr);
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

extern "C" __declspec(dllexport) void _stdcall SetAutoplay(HWND parent, int value, int speed)
{
	bool newValue = value != 0;
	if (g_autoplay == newValue)
		return;

	g_autoplay = newValue;

	if (g_autoplay)
	{
		EnsureTimerStopped();
		StartTimer(parent, 1, speed);
	}
	else
	{
		EnsureTimerStopped();
	}
}

extern "C" __declspec(dllexport) void _stdcall SetAutoplaySpeed(HWND parent, int speed)
{
	EnsureTimerStopped();
	StartTimer(parent, 1, speed);
}

extern "C" __declspec(dllexport) void _stdcall SaveGif(HWND parentDialog, int animationSpeed, int loopCount, int gifWidth, int gifHeight)
{
	TCHAR documentsPath[MAX_PATH];

	VerifyHR(SHGetFolderPath(NULL,
		CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
		NULL,
		0,
		documentsPath));

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	wchar_t szFile[MAX_PATH];
	wcscpy_s(szFile, L"export.gif");

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = parentDialog;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"Graphics Interchange Format (GIF)\0*.GIF\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = documentsPath;
	ofn.lpstrDefExt = L"txt";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn) == 0)
		return;

	EnsureWicImagingFactory();

	std::wstring destFilename = ofn.lpstrFile;

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
		if (loopCount > 0)
		{
			loopCountLsb = loopCount & 0xFF;
			loopCountMsb = loopCount >> 8;
		}

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
		gifWidth,
		gifHeight,
		GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnDemand, &singleSpriteWicBitmap)))
	{
		return;
	}

	ComPtr<ID2D1RenderTarget> singleSpriteWicBitmapRenderTarget;
	D2D1_RENDER_TARGET_PROPERTIES d2dRenderTargetProperties =
		D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

	if (FAILED(g_d2dFactory->CreateWicBitmapRenderTarget(singleSpriteWicBitmap.Get(), d2dRenderTargetProperties, &singleSpriteWicBitmapRenderTarget)))
	{
		return;
	}

	// Create a local copy of the spritesheet which is WIC target compatible, to sample from.
	ComPtr<ID2D1Bitmap> wicCompatibleSpritesheet;
	D2D1_BITMAP_PROPERTIES bitmapProperties = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

	// Create a wic bitmap depicting the loaded spritesheet
	/*
	if (g_d2dSpritesheetBitmap)
	{
		if (FAILED(singleSpriteWicBitmapRenderTarget->CreateBitmap(
			g_spritesheetBitmapData.Size,
			&g_spritesheetBitmapData.DestBuffer[0],
			g_spritesheetBitmapData.Size.width * sizeof(UINT),
			bitmapProperties,
			&wicCompatibleSpritesheet)))
		{
			return;
		}
	}


	int frameCount = static_cast<int>(g_spritesheetRects.size());

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

		if (FAILED(frameEncode->SetSize(gifWidth, gifHeight)))
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

		singleSpriteWicBitmapRenderTarget->BeginDraw();

		singleSpriteWicBitmapRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::CornflowerBlue));

		D2D1_RECT_F destRect = D2D1::RectF(0, 0, static_cast<float>(gifWidth), static_cast<float>(gifHeight));
		D2D1_RECT_F sourceRect = g_spritesheetRects[i];
		singleSpriteWicBitmapRenderTarget->DrawBitmap(
			wicCompatibleSpritesheet.Get(), 
			destRect, 
			1.0f, 
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, 
			sourceRect);

		if (FAILED(singleSpriteWicBitmapRenderTarget->EndDraw()))
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
	}*/
}

extern "C" __declspec(dllexport) void _stdcall ZoomIn(HWND parentDialog)
{
	if (g_zoomIndex >= _countof(g_zoomPercents) - 1)
		return;

	g_zoomIndex++;

	g_windowTitleHelper.SetZoomFactor(parentDialog, g_zoomPercentStrings[g_zoomIndex]);
}

extern "C" __declspec(dllexport) void _stdcall ZoomOut(HWND parentDialog)
{
	if (g_zoomIndex <= 0)
		return;

	g_zoomIndex--;

	g_windowTitleHelper.SetZoomFactor(parentDialog, g_zoomPercentStrings[g_zoomIndex]);
}

extern "C" __declspec(dllexport) void _stdcall ResetZoom(HWND parentDialog)
{
	g_zoomIndex = g_defaultZoomIndex;

	g_windowTitleHelper.SetZoomFactor(parentDialog, g_zoomPercentStrings[g_zoomIndex]);
}

extern "C" __declspec(dllexport) void _stdcall ReloadImage()
{
	LoadSpritesheetFileImpl();
}

extern "C" __declspec(dllexport) void _stdcall Uninitialize()
{
	g_d2dFactory.Reset();
	g_renderTarget.Reset();
	g_wicImagingFactory.Reset();
	g_colorizedFrames.clear();
}