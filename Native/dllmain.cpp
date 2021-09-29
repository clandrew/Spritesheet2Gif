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
ComPtr<ID2D1Bitmap> g_d2dSpritesheetBitmap;

GlobalAllocation g_svgDocumentAlloc;
ComPtr<ID2D1SvgDocument> g_svgDocument;
D2D1_SVG_VIEWBOX g_svgViewBox{};

std::vector<D2D1_RECT_F> g_spritesheetRects;
std::wstring g_spritesheetFilePath;
int g_spriteIndex;
int g_spriteWidth;
int g_spriteHeight;

struct WindowTitleHelper
{
	std::wstring m_programName;
	std::wstring m_currentFile;
	std::wstring m_zoomFactor;

	void UpdateWindowTitle(HWND dialogParent)
	{
		std::wstringstream strstrm;
		strstrm << m_programName.c_str();
		
		if (m_currentFile.size() > 0)
		{
			strstrm << L" - " << m_currentFile.c_str();
		}

		strstrm << L" - " << m_zoomFactor.c_str();

		SetWindowText(dialogParent, strstrm.str().c_str());
	}

public:
	void Initialize(HWND dialogParent, wchar_t const* zoomFactorString)
	{
		m_programName = L"Spritesheet2Gif";
		m_zoomFactor = zoomFactorString;
	}

	void SetZoomFactor(HWND dialogParent, wchar_t const* zoomFactorString)
	{
		m_zoomFactor = zoomFactorString;
		UpdateWindowTitle(dialogParent);
	}

	void SetOpenFileName(HWND dialogParent, std::wstring fullPath)
	{
		size_t delimiterIndex = g_spritesheetFilePath.rfind(L'\\');
		assert(delimiterIndex < g_spritesheetFilePath.size());
		if (delimiterIndex >= g_spritesheetFilePath.size())
			return;

		m_currentFile = g_spritesheetFilePath.substr(delimiterIndex + 1);
		UpdateWindowTitle(dialogParent);
	}
};

WindowTitleHelper g_windowTitleHelper;

static const float g_zoomPercents[] = {6.75f, 12.5f, 25, 50, 100, 200, 300, 400, 500, 600, 700, 800, 1000, 1200, 1400, 1600};
static const wchar_t* g_zoomPercentStrings[] = {L"6.75f%", L"12.5f%", L"25%", L"50%", L"100%", L"200%", L"300%", L"400%", L"500%", L"600%", L"700%", L"800%", L"1000%", L"1200%", L"1400%", L"1600"};

static const int g_defaultZoomIndex = 4;
int g_zoomIndex;

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

	g_spriteIndex = 0;
	g_autoplay = false;
	g_zoomIndex = g_defaultZoomIndex;

	g_windowTitleHelper.Initialize(parentDialog, g_zoomPercentStrings[g_zoomIndex]);
	
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
	D2D1_SIZE_F spritesheetSize{};
	float left = 0;
	float top = 0;

	if (g_d2dSpritesheetBitmap)
	{
		spritesheetSize = g_d2dSpritesheetBitmap->GetSize();
	}
	else if (g_svgDocument)
	{
		spritesheetSize = D2D1::SizeF(g_svgViewBox.width, g_svgViewBox.height);
		left = g_svgViewBox.x;
		top = g_svgViewBox.y;
	}
	else
	{
		return;
	}

	g_spritesheetRects.clear();

	for (float y = top; y < spritesheetSize.height; y += (float)g_spriteHeight)
	{
		for (float x = 0; x < spritesheetSize.width; x += static_cast<float>(g_spriteWidth))
		{
			D2D1_RECT_F rect;

			rect.left = x;
			rect.right = rect.left + static_cast<float>(g_spriteWidth);
			rect.right = min(rect.right, spritesheetSize.width);

			rect.top = y;
			rect.bottom = rect.top + static_cast<float>(g_spriteHeight);
			rect.bottom = min(rect.bottom, spritesheetSize.height);

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

void TryLoadAsSvg(std::wstring svgFilePath)
{
	FILE* file{};
	if (_wfopen_s(&file, svgFilePath.c_str(), L"rb") != 0)
	{
		return;
	}

	if (fseek(file, 0, SEEK_END) != 0)
	{
		return;
	}

	long fileLength = ftell(file);

	if (fileLength == 0)
	{
		return;
	}

	g_svgDocumentAlloc.Initialize(fileLength);

	if (fseek(file, 0, SEEK_SET) != 0)
	{
		return;
	}

	if (fread(g_svgDocumentAlloc.m_locked, 1, fileLength, file) != fileLength)
	{
		return;
	}

	if (fclose(file) != 0)
	{
		return;
	}

	if (FAILED(CreateStreamOnHGlobal(g_svgDocumentAlloc.m_handle, TRUE, &g_svgDocumentAlloc.m_stream)))
	{
		return;
	}

	ComPtr<ID2D1DeviceContext5> deviceContext5;
	if (FAILED(g_renderTarget.As(&deviceContext5)))
	{
		return;
	}

	if (FAILED(deviceContext5->CreateSvgDocument(
		g_svgDocumentAlloc.m_stream.Get(),
		D2D1::SizeF(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()),
		&g_svgDocument)))
	{
		return;
	}

	ComPtr<ID2D1SvgElement> rootNode;
	g_svgDocument->GetRoot(&rootNode);

	if (FAILED(rootNode->GetAttributeValue(L"viewBox", D2D1_SVG_ATTRIBUTE_POD_TYPE_VIEWBOX, &g_svgViewBox, sizeof(g_svgViewBox))))
	{
		return;
	}
}

bool TryLoadAsRaster(std::wstring fileName)
{
	ComPtr<IWICBitmapDecoder> spDecoder;
	if (FAILED(g_wicImagingFactory->CreateDecoderFromFilename(
		g_spritesheetFilePath.c_str(),
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

	if (FAILED(spConverter->GetSize(&g_spritesheetBitmapData.Size.width, &g_spritesheetBitmapData.Size.height)))
	{
		return false;
	}

	g_spritesheetBitmapData.DestBuffer.resize(g_spritesheetBitmapData.Size.width * g_spritesheetBitmapData.Size.height);
	if (FAILED(spConverter->CopyPixels(
		NULL,
		g_spritesheetBitmapData.Size.width * sizeof(UINT),
		static_cast<UINT>(g_spritesheetBitmapData.DestBuffer.size()) * sizeof(UINT),
		(BYTE*)&g_spritesheetBitmapData.DestBuffer[0])))
	{
		return false;
	}

	D2D1_BITMAP_PROPERTIES bitmapProperties = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
	if (FAILED(g_renderTarget->CreateBitmap(
		g_spritesheetBitmapData.Size,
		&g_spritesheetBitmapData.DestBuffer[0],
		g_spritesheetBitmapData.Size.width * sizeof(UINT),
		bitmapProperties,
		&g_d2dSpritesheetBitmap)))
	{
		return false;
	}

	return true;
}

void LoadSpritesheetFileImpl()
{
	EnsureWicImagingFactory();

	if (!TryLoadAsRaster(g_spritesheetFilePath.c_str()))
	{
		TryLoadAsSvg(g_spritesheetFilePath.c_str());
	}

	return;

}

void OpenSpritesheetFileImpl(HWND dialogParent, std::wstring fileName)
{
	g_spritesheetFilePath = fileName;

	LoadSpritesheetFileImpl();

	if (g_d2dSpritesheetBitmap)
	{
		g_spriteWidth = g_spritesheetBitmapData.Size.width;
		g_spriteHeight = g_spritesheetBitmapData.Size.height;
	}
	else if (g_svgDocument)
	{
		g_spriteWidth = g_svgViewBox.width;
		g_spriteHeight = g_svgViewBox.height;
	}

	UpdateSpritesheetRects();

	g_windowTitleHelper.SetOpenFileName(dialogParent, g_spritesheetFilePath);

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
	std::wstring testFilename = L"D:\\repos\\Spritesheet2Gif\\Debug\\bunny.svg";

	OpenSpritesheetFileImpl(dialogParent, testFilename);
}
#endif

extern "C" __declspec(dllexport) void _stdcall Paint()
{
	g_renderTarget->BeginDraw();
	g_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	D2D1_RECT_F sourceRect = g_spritesheetRects[g_spriteIndex];

	float zoomFactor = g_zoomPercents[g_zoomIndex] / 100.0f;

	if (g_d2dSpritesheetBitmap)
	{
		D2D1_MATRIX_3X2_F transform = D2D1::Matrix3x2F::Scale(zoomFactor, zoomFactor);
		g_renderTarget->SetTransform(transform);

		D2D1_SIZE_F spritesheetSize = g_d2dSpritesheetBitmap->GetSize();
		
		D2D1_RECT_F destRect = D2D1::RectF(0, 0, static_cast<float>(g_spriteWidth), static_cast<float>(g_spriteHeight));
		g_renderTarget->DrawBitmap(
			g_d2dSpritesheetBitmap.Get(), 
			destRect, 
			1.0f, 
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, 
			sourceRect);
	}
	else if (g_svgDocument)
	{
		ComPtr<ID2D1DeviceContext5> deviceContext5;
		if (FAILED(g_renderTarget.As(&deviceContext5)))
		{
			return;
		}

		D2D1_MATRIX_3X2_F transform;
		transform = D2D1::Matrix3x2F::Translation(D2D1::SizeF(-sourceRect.left, -sourceRect.top)) * D2D1::Matrix3x2F::Scale(zoomFactor, zoomFactor);	
		deviceContext5->SetTransform(transform);

		D2D1_RECT_F clipRect = sourceRect;
		deviceContext5->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_ALIASED);

		deviceContext5->DrawSvgDocument(g_svgDocument.Get());

		deviceContext5->PopAxisAlignedClip();

		deviceContext5->SetTransform(D2D1::Matrix3x2F::Identity());
	}

	VerifyHR(g_renderTarget->EndDraw());	
}

extern "C" __declspec(dllexport) int _stdcall GetSpritesheetWidth()
{
	if (g_d2dSpritesheetBitmap)
	{
		D2D1_SIZE_F spritesheetSize = g_d2dSpritesheetBitmap->GetSize();
		return static_cast<int>(spritesheetSize.width);
	}
	else if (g_svgDocument)
	{
		return static_cast<int>(g_svgViewBox.width);
	}
	else
	{
		return 0;
	}
}

extern "C" __declspec(dllexport) int _stdcall GetSpritesheetHeight()
{
	if (g_d2dSpritesheetBitmap)
	{
		D2D1_SIZE_F spritesheetSize = g_d2dSpritesheetBitmap->GetSize();
		return static_cast<int>(spritesheetSize.height);
	}
	else if (g_svgDocument)
	{
		return static_cast<int>(g_svgViewBox.height);
	}
	else
	{
		return 0;
	}
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
		g_spriteIndex = static_cast<int>(g_spritesheetRects.size()) - 1;
	}
}

extern "C" __declspec(dllexport) void _stdcall NextSprite()
{
    assert(g_spritesheetRects.size() <= INT_MAX);
	if (g_spriteIndex < static_cast<int>(g_spritesheetRects.size()) - 1)
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
    UINT_PTR idTimer,
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
	if (g_svgDocument)
	{
		float spritesheetWidth = g_svgViewBox.width;
		float spritesheetHeight = g_svgViewBox.height;

		ComPtr<ID2D1DeviceContext> deviceContext6;
		g_renderTarget.As(&deviceContext6);

		D2D1_BITMAP_PROPERTIES1 lockableProperties = D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
		ComPtr<ID2D1Bitmap1> lockable;
		if (FAILED(deviceContext6->CreateBitmap(D2D1::SizeU(spritesheetWidth, spritesheetHeight), nullptr, 0, lockableProperties, &lockable)))
		{
			return;
		}
		g_renderTarget->BeginDraw();
		g_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		ComPtr<ID2D1DeviceContext5> deviceContext5;
		if (FAILED(g_renderTarget.As(&deviceContext5)))
		{
			return;
		}

		D2D1_MATRIX_3X2_F transform;
		transform = D2D1::Matrix3x2F::Translation(D2D1::SizeF(-g_svgViewBox.x, g_svgViewBox.y));
		deviceContext5->SetTransform(transform);

		D2D1_RECT_F clipRect = D2D1::RectF(g_svgViewBox.x, g_svgViewBox.y, g_svgViewBox.width, g_svgViewBox.height);
		deviceContext5->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_ALIASED);

		deviceContext5->DrawSvgDocument(g_svgDocument.Get());

		deviceContext5->PopAxisAlignedClip();

		deviceContext5->SetTransform(D2D1::Matrix3x2F::Identity());

		g_renderTarget->EndDraw();

		D2D1_POINT_2U destPoint = D2D1::Point2U(0, 0);
		D2D1_RECT_U sourceRect = D2D1::RectU(g_svgViewBox.x, g_svgViewBox.y, g_svgViewBox.width, g_svgViewBox.height);
		if (FAILED(lockable->CopyFromRenderTarget(&destPoint, g_renderTarget.Get(), &sourceRect)))
		{
			return;
		}

		D2D1_MAPPED_RECT mappedRect{};
		if (FAILED(lockable->Map(D2D1_MAP_OPTIONS_READ, &mappedRect)))
		{
			return;
		}

		D2D1_SIZE_U spritesheetSizeU = D2D1::SizeU(spritesheetWidth, spritesheetHeight);

		if (FAILED(singleSpriteWicBitmapRenderTarget->CreateBitmap(
			spritesheetSizeU,
			mappedRect.bits,
			mappedRect.pitch,
			bitmapProperties,
			&wicCompatibleSpritesheet)))
		{
			return;
		}

		if (FAILED(lockable->Unmap()))
		{
			return;
		}

		Paint();
		
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
	}
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
	g_svgDocumentAlloc.m_stream.Reset(); // Important- delete the SVG structures before the factory.
	g_svgDocument.Reset();

	g_d2dFactory.Reset();
	g_renderTarget.Reset();
	g_d2dSpritesheetBitmap.Reset();
	g_wicImagingFactory.Reset();
}