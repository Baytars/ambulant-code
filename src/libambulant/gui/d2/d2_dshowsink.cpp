//needed for DirectShow. Do not change order of the first 3 includes!
//#include <streams.h>

#include "ambulant/gui/d2/d2_dshowsink.h"
#include "ambulant/lib/logger.h"

#include <d2d1.h>
#include <d2d1helper.h>

#include <cassert>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

//for unicode
//#include "tchar.h"

//Every Direct3D application this
//#include <d3d9.h>
//#include <d3dx9.h>

//To do error handling. Do not forget to link to dxerr.lib! 
//#include <dxerr.h>

struct __declspec(uuid("{AB1B2AB5-18A0-49D5-814F-E2CB454D5D28}")) CLSID_TextureRenderer;

//Text Buffer
static TCHAR g_tcGeneralTxtBuffer[MAX_PATH];

#define SAFE_RELEASE(var) \
	if(var!=NULL)\
{\
	var->Release();\
	var = NULL;\
}

#define HRESULT_CHECK(hr)\
	do {\
	if(FAILED(hr))\
		{\
		_stprintf_s(g_tcGeneralTxtBuffer,TEXT("ERROR: %s. %s!"), DXGetErrorString(hr), DXGetErrorDescription(hr));\
		MessageBox( NULL, g_tcGeneralTxtBuffer, "ERROR", MB_ICONERROR | MB_OK );\
		}\
	} while(0)

#ifdef JNK
bool g_bContinue = true;

LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;

LPD3DXEFFECT g_lpEffect = NULL;
LPDIRECT3DVERTEXBUFFER9 g_lpVertexBuffer = NULL;

LPDIRECT3DTEXTURE9 g_lpTexture = NULL;

//The needed DirectShow Interfaces.
IGraphBuilder *g_pGraph;
IMediaControl *g_pControl;
IMediaEventEx *g_pEvent;
IMediaSeeking *g_pMediaSeeking;
IBaseFilter   *g_pRenderer;    // our custom renderer

//Here we define a new WindowsMessage Identifier.
//Applications can use message numbers in the range from 
//WM_APP through 0xBFFF as private messages
#define WM_GRAPHNOTIFY  WM_APP + 1

//The combined transformation will end up in this matrix:
D3DXMATRIX g_ShaderMatrix;
//Camera Position
D3DXVECTOR3 g_v3Position = D3DXVECTOR3(0.0f,0.0f,-5.0f);
//Camera LookAt point
D3DXVECTOR3 g_v3LookAt = D3DXVECTOR3(0.0f,0.0f,0.0f);

//Definition of the first Vertex Format 
//including position diffuse color and now texture coordinates.
#define D3DFVF_COLOREDVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE1(2))

struct COLORED_VERTEX
{
	float x, y, z;	//Position
	DWORD color;	//Color
	float u,v;      //Texture Coordinates
};

#endif // JNK

/// <summary>
/// The new class.
/// An implementation of a custom VideoRenderer
/// </summary>

//-----------------------------------------------------------------------------
// CVideoTextureRenderer constructor
//-----------------------------------------------------------------------------
CVideoD2DBitmapRenderer::CVideoD2DBitmapRenderer(LPUNKNOWN pUnk, HRESULT *phr)
:	CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), NAME("Texture Renderer"), pUnk, phr),
	m_rt(NULL),
	m_d2bitmap(NULL),
//	m_d2bitmap_next(NULL),
	m_callback(NULL),
	m_width(0),
	m_height(0),
	m_pitch(0),
	m_has_alpha(false),
	m_ignore_timestamps(false)
{
#ifdef JNK
	m_bUseDynamicTextures = FALSE;

	m_ppVideoDestTexture = NULL;
	m_lpVideoDestSurface = NULL;

	m_lpVideoTargetTexture = NULL;
	m_lpVideoTargetSurface = NULL;
#endif // JNK
	*phr = S_OK;
}


//-----------------------------------------------------------------------------
// CVideoTextureRenderer destructor
//-----------------------------------------------------------------------------
CVideoD2DBitmapRenderer::~CVideoD2DBitmapRenderer()
{
#if 0
	if (m_d2bitmap_next && m_d2bitmap_next != m_d2bitmap) {
		m_d2bitmap_next->Release();
		m_d2bitmap_next = NULL;
	}
#endif
	if (m_d2bitmap) {
		m_d2bitmap->Release();
		m_d2bitmap = NULL;
	}
	m_rt = NULL;
#ifdef JNK
	//Clean Up
	SAFE_RELEASE((*m_ppVideoDestTexture));
	SAFE_RELEASE(m_lpVideoDestSurface);

	if(m_bUseDynamicTextures)
	{
		SAFE_RELEASE(m_lpVideoTargetTexture);
		SAFE_RELEASE(m_lpVideoTargetSurface);
	}
#endif // JNK
}

void
CVideoD2DBitmapRenderer::SetRenderTarget(ID2D1RenderTarget *rt)
{
	m_rt = rt;
}

void
CVideoD2DBitmapRenderer::SetCallback(IVideoD2DBitmapRendererCallback *callback)
{
	m_callback = callback;
}

ID2D1Bitmap *
CVideoD2DBitmapRenderer::LockBitmap()
{
	// XXX Lock it.
//	assert(m_d2bitmap_next == NULL);
	return m_d2bitmap;
}

void
CVideoD2DBitmapRenderer::UnlockBitmap()
{
#if 0
	if (m_d2bitmap != m_d2bitmap_next) {
		// A new one has arrived, in the mean time.
		if (m_d2bitmap) m_d2bitmap->Release();
		m_d2bitmap = m_d2bitmap_next;
		m_d2bitmap_next = NULL;
	}
#endif
	// XXX Unlock it.
}

void
CVideoD2DBitmapRenderer::DestroyBitmap()
{
	if (m_d2bitmap) m_d2bitmap->Release();
	m_d2bitmap = NULL;
}

//-----------------------------------------------------------------------------
// CheckMediaType: This method forces the graph to give us an R8G8B8 video
// type, making our copy to texture memory trivial.
//-----------------------------------------------------------------------------
HRESULT CVideoD2DBitmapRenderer::CheckMediaType(const CMediaType *pmt)
{
	HRESULT hr = E_FAIL;
	VIDEOINFO *pvi=0;

	CheckPointer(pmt,E_POINTER);

	// Reject the connection if this is not a video type
	if (*pmt->FormatType() != FORMAT_VideoInfo ) {
		return E_INVALIDARG;
	}

	// Only accept RGB24 video
	pvi = (VIDEOINFO *)pmt->Format();

	if (IsEqualGUID(*pmt->Type(), MEDIATYPE_Video)) {
		if (IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_RGB24)) {
			AM_DBG ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::CheckMediaType: MEDIASUBTYPE_RGB24");
			hr = S_OK;
		}
		if (IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_RGB32)) {
			AM_DBG ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::CheckMediaType: MEDIASUBTYPE_RGB32");
			hr = S_OK;
		}
		if (IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_ARGB32)) {
			AM_DBG ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::CheckMediaType: MEDIASUBTYPE_ARGB32");
			hr = S_OK;
		}
	}
	return hr;
}

//-----------------------------------------------------------------------------
// SetMediaType: Graph connection has been made.
//-----------------------------------------------------------------------------
HRESULT CVideoD2DBitmapRenderer::SetMediaType(const CMediaType *pmt)
{
	HRESULT hr;

	UINT uintWidth = 2;
	UINT uintHeight = 2;

	// Retrieve the size of this media type
//JNK	D3DCAPS9 caps;
	assert(IsEqualGUID(*pmt->Type(), MEDIATYPE_Video));
	if (IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_RGB32)) {
		AM_DBG ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::SetMediaType: MEDIASUBTYPE_RGB32");
		hr = S_OK;
	} else if (IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_ARGB32)) {
		AM_DBG ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::SetMediaType: MEDIASUBTYPE_ARGB32");
		hr = S_OK;
	} else {
		assert(0);
	}
	VIDEOINFO *pviBmp;                      // Bitmap info header
	pviBmp = (VIDEOINFO *)pmt->Format();

	m_width = pviBmp->bmiHeader.biWidth;
	m_height = abs(pviBmp->bmiHeader.biHeight);
	m_pitch = m_width*4; // Only 32-bit formats supported.
	m_has_alpha = MEDIASUBTYPE_HASALPHA(*pmt);

#ifdef JNK
	// here let's check if we can use dynamic textures
	ZeroMemory( &caps, sizeof(D3DCAPS9));
	hr = g_pd3dDevice->GetDeviceCaps( &caps );
	if( caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES )
	{
		m_bUseDynamicTextures = TRUE;
	}

	//check for power of 2 texture requirement
	if( caps.TextureCaps & D3DPTEXTURECAPS_POW2 )
	{
		while( (LONG)uintWidth < m_lVidWidth )
		{
			uintWidth = uintWidth << 1;
		}
		while( (LONG)uintHeight < m_lVidHeight )
		{
			uintHeight = uintHeight << 1;
		}
	}
	else
	{
		uintWidth = m_lVidWidth;
		uintHeight = m_lVidHeight;
	}

	// Create the texture that maps to this media type
	hr = E_UNEXPECTED;
	if( m_bUseDynamicTextures )
	{
		hr = g_pd3dDevice->CreateTexture(uintWidth, uintHeight, 1, D3DUSAGE_DYNAMIC,
			D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_lpVideoTargetTexture, NULL);

		if( FAILED(hr))
		{
			m_bUseDynamicTextures = FALSE;
		}
		else
		{
			//if you use the texture, you render the video to(m_lpVideoTargetTexture) 
			//directly in your main loop, you may encounter a hang up in your Device->Present() Method. 
			//To be safe we create a second texture for the main loop(m_ppVideoDestTexture == g_lpTexture) and update this one via StretchRect() in the 
			//DoRenderSample().
			hr = g_pd3dDevice->CreateTexture(uintWidth, uintHeight, 1, D3DUSAGE_RENDERTARGET,
				D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT, m_ppVideoDestTexture, NULL);
			if( FAILED(hr))
			{
				return hr;
			}
			//StretchRect() only works for surfaces, so get the 2 surfaces here
			hr = m_lpVideoTargetTexture->GetSurfaceLevel(0, &m_lpVideoTargetSurface);
			hr = (*m_ppVideoDestTexture)->GetSurfaceLevel(0, &m_lpVideoDestSurface);
		}
	}
	if( FALSE == m_bUseDynamicTextures )
	{
		hr = g_pd3dDevice->CreateTexture(uintWidth, uintHeight, 1, 0,
			D3DFMT_X8R8G8B8,D3DPOOL_MANAGED, m_ppVideoDestTexture, NULL);
		if( FAILED(hr))
		{
			return hr;
		}
		m_lpVideoTargetTexture = *m_ppVideoDestTexture;
	}
#endif // JNK
	return S_OK;
}


//-----------------------------------------------------------------------------
// DoRenderSample: A sample has been delivered. Copy it to the texture.
//-----------------------------------------------------------------------------
HRESULT CVideoD2DBitmapRenderer::DoRenderSample( IMediaSample * pSample )
{
	HRESULT hr;
	BYTE  *pBmpBuffer;
//JNK	BYTE *pTexBuffer; // Bitmap buffer, texture buffer
//JNK	LONG  lTexPitch;                // Pitch of bitmap, texture

	BYTE  * pbS = NULL;
	BYTE * pBMPBytes = NULL;
	BYTE * pTextureBytes = NULL;
//JNK	UINT row, col, dwordWidth;

	CheckPointer(pSample,E_POINTER);
//JNK	CheckPointer(m_lpVideoTargetTexture,E_UNEXPECTED);

	// Get the video bitmap buffer
	pSample->GetPointer( &pBmpBuffer );
	AM_DBG ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::DoRenderSample() called");
	if (m_rt == NULL) 
		return S_OK;

	ID2D1Bitmap *bitmap = NULL;
	D2D1_SIZE_U size = D2D1::SizeU(m_width, m_height);
	D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties();
	props.pixelFormat = D2D1::PixelFormat(
		DXGI_FORMAT_B8G8R8A8_UNORM,
		m_has_alpha ? D2D1_ALPHA_MODE_PREMULTIPLIED : D2D1_ALPHA_MODE_IGNORE);
	hr = m_rt->CreateBitmap(size, pBmpBuffer, m_pitch, props, &bitmap);
	if (!SUCCEEDED(hr)) {
		ambulant::lib::logger::get_logger()->trace("CVideoD2DBitmapRenderer::DoRenderSample: CreateBitmap: error 0x%x", hr);
	}
	// XXX Lock
	AM_DBG ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::DoRenderSample: new bitmap is 0x%x", bitmap);
	ID2D1Bitmap *old_bitmap = m_d2bitmap;
	m_d2bitmap = bitmap;
	// XXX Unlock
	if (old_bitmap) 
		old_bitmap->Release();
	if (m_callback) 
		m_callback->BitmapAvailable(this);
#ifdef JNK
	// Lock the Texture
	D3DLOCKED_RECT d3dlr;
	if( m_bUseDynamicTextures )
	{
		if( FAILED(m_lpVideoTargetTexture->LockRect(0, &d3dlr, 0, D3DLOCK_DISCARD)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_lpVideoTargetTexture->LockRect(0, &d3dlr, 0, 0)))
			return E_FAIL;
	}
	// Get the texture buffer & pitch
	pTexBuffer = static_cast<byte *>(d3dlr.pBits);
	lTexPitch = d3dlr.Pitch;

	//bitmaps are stored upside-down so we have to start from the last row here.
	UINT uiStartOffset = lTexPitch * (m_lVidHeight-1);
	pTexBuffer = pTexBuffer+uiStartOffset;
	// (pixel by 3 bytes over sizeof(DWORD))
	for( row = 0; row < (UINT)m_lVidHeight; row++)
	{
		pBMPBytes = pBmpBuffer;
		pTextureBytes = pTexBuffer;

		for( col = 0; col < m_lVidWidth; col ++ )
		{
			//copying from RGB to RGBA(== D3DFMT_X8R8G8B8 of texture)
			pTextureBytes[0] = pBMPBytes[0];//r
			pTextureBytes[1] = pBMPBytes[1];//g
			pTextureBytes[2] = pBMPBytes[2];//b
			//because we have X8R8G8B8 alpha is useless here.
			//But you can try the D3DFMT_A8R8G8B8 and set another alpha value here
			//so you can render a semi-transparent movie if you like!
			pTextureBytes[3] = 0xff;//a

			//Texture uses 4 bytes (=32bit Format D3DFMT_X8R8G8B8)
			pTextureBytes +=4;
			//Bitmap(Video) uses 3 bytes (=24bit MEDIASUBTYPE_RGB24 )
			pBMPBytes +=3;
		}

		pBmpBuffer  += m_lVidPitch;
		pTexBuffer -= lTexPitch;
	}

	// Unlock the Texture
	if (FAILED(m_lpVideoTargetTexture->UnlockRect(0)))
		return E_FAIL;

	if(m_bUseDynamicTextures)
	{
		//When using a dynamic texture use this method to copy from your Video rendertarget to your 
		//application surface(texture).
		g_pd3dDevice->StretchRect(m_lpVideoTargetSurface, NULL, m_lpVideoDestSurface, NULL,  D3DTEXF_NONE);
	}
#endif // JNK
	return S_OK;
}

HRESULT CVideoD2DBitmapRenderer::ShouldDrawSampleNow(IMediaSample *pMediaSample,
                                                __inout REFERENCE_TIME *ptrStart,
                                                __inout REFERENCE_TIME *ptrEnd)
{
	HRESULT rv;
	if (m_ignore_timestamps) {
		rv = S_OK;
	} else {
		rv = CBaseVideoRenderer::ShouldDrawSampleNow(pMediaSample, ptrStart, ptrEnd);
	}
	AM_DBG ambulant::lib::logger::get_logger()->debug("ShouldDrawSampleNow(..., %lld, %lld) [ignoretimestamp=%d] -> 0x%x", (long long)*ptrStart, (long long)*ptrEnd, m_ignore_timestamps, rv);
	return rv;
}

#ifdef JNK
void CVideoTextureRenderer::SetVideoTexture(LPDIRECT3DTEXTURE9* ppTexture)
{
	m_ppVideoDestTexture = ppTexture;
}
#endif // JNK

#ifdef JNK
//Sets up DirectShow to render to the specified texture
void CreateVideoTexture( LPDIRECT3DDEVICE9 g_pd3dDevice, HWND hWnd, LPCWSTR path, BOOL bRepeat, LPDIRECT3DTEXTURE9* ppTexture )
{
	HRESULT hr = S_OK;
	IBaseFilter    *pFSrc;          // Source Filter
	IPin           *pFSrcPinOut;    // Source Filter Output Pin
	CVideoTextureRenderer *pVideoTextureRenderer=0; // DirectShow Texture renderer

	// Create the filter graph
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&g_pGraph);

	// Create the Texture Renderer object
	pVideoTextureRenderer = new CVideoTextureRenderer(NULL, &hr);
	pVideoTextureRenderer->SetVideoTexture(ppTexture);

	// Get a pointer to the IBaseFilter on the TextureRenderer, add it to graph
	g_pRenderer = pVideoTextureRenderer;
	hr = g_pGraph->AddFilter(g_pRenderer, L"TEXTURERENDERER");

	// Add the source filter to the graph.
	hr = g_pGraph->AddSourceFilter (path, L"SOURCE", &pFSrc);

	hr = pFSrc->FindPin(L"Output", &pFSrcPinOut);

	// Render the source filter's output pin.  The Filter Graph Manager
	// will connect the video stream to the loaded CVideoTextureRenderer
	// and will load and connect an audio renderer (if needed).
	hr = g_pGraph->Render(pFSrcPinOut);

	// Get the graph's media control, event & position interfaces
	hr = g_pGraph->QueryInterface(IID_IMediaControl, (void **)&g_pControl);
	hr = g_pGraph->QueryInterface(IID_IMediaEventEx, (void **)&g_pEvent);
	hr = g_pGraph->QueryInterface(IID_IMediaSeeking, (void **)&g_pMediaSeeking);

	if(bRepeat)
		g_pEvent->SetNotifyWindow((OAHWND)hWnd, WM_GRAPHNOTIFY, 0);

	// Start the graph running;
	hr = g_pControl->Run();

}


//Besides the main function, there must be a message processing function
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	float fSpeed = 0.2f;
	switch( msg )
	{
	case WM_DESTROY:
		PostQuitMessage( 0 );
		g_bContinue = false;
		return 0;
		//handle key press
	case WM_KEYDOWN:
		switch(wParam)
		{
			//change lookAt with arrow keys
		case VK_UP:
			g_v3LookAt.y += fSpeed;
			break;
		case VK_DOWN:
			g_v3LookAt.y -= fSpeed;
			break;
		case VK_LEFT:
			g_v3LookAt.x += fSpeed;
			break;
		case VK_RIGHT:
			g_v3LookAt.x -= fSpeed;
			break;
		}
		break;
	//when repeat was set to TRUE when calling
	//CreateVideoTexture() this message will be processed
	case WM_GRAPHNOTIFY:
		{
			// Disregard if we don't have an IMediaEventEx pointer.
			if (g_pEvent == NULL)
			{
				break;
			}
			// Get all the events
			long evCode;
			LONG_PTR param1, param2;
			HRESULT hr;
			while (SUCCEEDED(g_pEvent->GetEvent(&evCode, &param1, &param2, 0)))
			{
				g_pEvent->FreeEventParams(evCode, param1, param2);
				switch (evCode)
				{
				case EC_COMPLETE: //file/movie was played completely
					{
						//repeat playing = set absolute(AM_SEEKING_AbsolutePositioning) position to the beginning = 0.
						REFERENCE_TIME currentPos  = 0;
						if(FAILED(g_pMediaSeeking->SetPositions( &currentPos,  AM_SEEKING_AbsolutePositioning, 
							NULL, AM_SEEKING_NoPositioning )))
							int a=0;
						break;
					}
				case EC_USERABORT: // Fall through.
				case EC_ERRORABORT:
					int a=0;
					break;
				}
			} 
		}
		break;
	}
	return DefWindowProc( hWnd, msg, wParam, lParam );
}

//The entry point of a windows application is the WinMain function
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
	HRESULT hr;

	//Create a window class.
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		"Direct3D Window", NULL };
	//Register the window class.
	RegisterClassEx( &wc );
	//Create the application's window.
	HWND hWnd = CreateWindow( "Direct3D Window", "DirectX Wiki - D3D9 Tutorial 5", 
		WS_OVERLAPPEDWINDOW, 100, 100, 400, 400,
		GetDesktopWindow(), NULL, wc.hInstance, NULL );

	ShowWindow(hWnd,SW_SHOW);
	//Create the Direct3D Object
	LPDIRECT3D9 pD3D = NULL;
	if( NULL == (pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;
	//Setup the device presentation parameters
	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	//The final step is to use the IDirect3D9::CreateDevice method to create the Direct3D device, 
	//as illustrated in the following code example.
	if( FAILED( pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED,
		&d3dpp, &g_pd3dDevice ) ) )
	{
		MessageBox(hWnd, "No HAL HARDWARE_VERTEXPROCESSING! Sample will exit!", NULL, 0);
		pD3D->Release();
		pD3D = NULL;
		return E_FAIL;
	}

	//setting the  buffer size to 4 vertices * structure size
	UINT uiBufferSize = 4*sizeof(COLORED_VERTEX);
	//creating the Vertexbuffer
	if( FAILED( g_pd3dDevice->CreateVertexBuffer( uiBufferSize,
		D3DUSAGE_WRITEONLY, D3DFVF_COLOREDVERTEX, D3DPOOL_DEFAULT, &g_lpVertexBuffer, NULL ) ) )
		return E_FAIL;

	COLORED_VERTEX* pVertices;
	//lock for writing.
	if( FAILED( g_lpVertexBuffer->Lock( 0, uiBufferSize, (void**)&pVertices, 0 ) ) )
		return E_FAIL;

	//Write now.
	//A simple Quadrangle
	pVertices[0].x =  -1.0f; //left
	pVertices[0].y =  -1.0f; //bottom
	pVertices[0].z =   0.0f;
	pVertices[0].color =  0xffff0000; //red
	//set texture coordinates. But remember the origin is top-left
	pVertices[0].u = 0.0f;
	pVertices[0].v = 1.0f;

	pVertices[1].x =  -1.0f; //left
	pVertices[1].y =   1.0f; //top
	pVertices[1].z =   0.0f;
	pVertices[1].color =  0xff0000ff; //blue
	pVertices[1].u = 0.0f;
	pVertices[1].v = 0.0f;

	pVertices[2].x =   1.0f; //right
	pVertices[2].y =  -1.0f; //bottom
	pVertices[2].z =   0.0f;
	pVertices[2].color =  0xff00ff00; //green
	pVertices[2].u = 1.0f;
	pVertices[2].v = 1.0f;

	pVertices[3].x =  1.0f; //right
	pVertices[3].y =  1.0f; //top 
	pVertices[3].z =  0.0f;
	pVertices[3].color =  0xffffffff; //white
	pVertices[3].u = 1.0f;
	pVertices[3].v = 0.0f;

	//unlock buffer.
	g_lpVertexBuffer->Unlock();

	//setting the vertex formats
	g_pd3dDevice->SetFVF( D3DFVF_COLOREDVERTEX );
	//handover the buffer to the device
	g_pd3dDevice->SetStreamSource( 0, g_lpVertexBuffer, 0, sizeof(COLORED_VERTEX) );

	if(FAILED(D3DXCreateEffectFromFile( g_pd3dDevice, "Effect.fx", NULL, 
		NULL, D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY, NULL, &g_lpEffect, NULL )))
		return E_FAIL;

	//create the texture from a movie file
	CreateVideoTexture(g_pd3dDevice, hWnd, L"higuita.avi", TRUE, &g_lpTexture);

	//the matrices:
	//the first transforms from world to view/camera space, 
	//the second from camera to screen space, 
	//the third transforms from object to world space
	D3DXMATRIX ViewMatrix, PerspectiveMatrix, WorldMatrix;
	//we initialize them with identity
	D3DXMatrixIdentity(&g_ShaderMatrix);
	D3DXMatrixIdentity(&WorldMatrix);
	D3DXMatrixIdentity(&ViewMatrix);
	D3DXMatrixIdentity(&PerspectiveMatrix);
	//calculating a perspective projection matrix
	//parameters besides the output matrix are:
	//the fovY, the aspect ration, the near and far z values(for clipping)
	D3DXMatrixPerspectiveFovLH(&PerspectiveMatrix, D3DX_PI/4.0f, 1.0f, 0.01f, 100.0f);

	//handover the texture to the effect
	g_lpEffect->SetTexture("ColorMap", g_lpTexture);

	MSG msg; 
	while( g_bContinue )
	{
		//Do a little position animation here for the objects world matrix
		D3DXMatrixRotationY(&WorldMatrix,GetTickCount()/1000.0f); 
		//Calculate a view matrix with position and look at vector
		D3DXMatrixLookAtLH(&ViewMatrix, &g_v3Position, &g_v3LookAt, &D3DXVECTOR3(0.0f,1.0f,0.0f));
		//Concatenating the matrices by multiplication.
		g_ShaderMatrix = WorldMatrix * ViewMatrix * PerspectiveMatrix;
		//handover the matrix to the effect.
		g_lpEffect->SetMatrix( "ShaderMatrix", &g_ShaderMatrix );

		//g_pd3dDevice->SetRenderTarget(0, g_lpMainRenderTarget);
		//Clear render region with blue
		hr = g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );
		//before rendering something, you have to call this
		hr = g_pd3dDevice->BeginScene();
		HRESULT_CHECK(hr);


		//rendering of scene objects happens here
		UINT uiPasses = 0;
		hr = g_lpEffect->Begin(&uiPasses, 0);
		HRESULT_CHECK(hr);
		for (UINT uiPass = 0; uiPass < uiPasses; uiPass++)
		{
			g_lpEffect->BeginPass(uiPass);

			//drawing the quad
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

			g_lpEffect->EndPass();
		}
		g_lpEffect->End();

		//after the scene call
		hr = g_pd3dDevice->EndScene();

		//update Screen
		hr = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
		HRESULT_CHECK(hr);

		// A window has to handle its messages.
		TranslateMessage( &msg );
		DispatchMessage( &msg );
		PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
	}

	//cleanup dshow
	g_pControl->Stop();
	SAFE_RELEASE(g_pControl);
	SAFE_RELEASE(g_pMediaSeeking);
	g_pEvent->SetNotifyWindow(NULL, 0, 0);
	SAFE_RELEASE(g_pEvent);
	//Release filters. Enumerate the filters in the graph.
	IEnumFilters *pEnum = NULL;
	hr = g_pGraph->EnumFilters(&pEnum);
	if (SUCCEEDED(hr))
	{
		IBaseFilter *pFilter = NULL;
		while (S_OK == pEnum->Next(1, &pFilter, NULL))
		{
			// Remove the filter.
			g_pGraph->RemoveFilter(pFilter);
			// Reset the enumerator.
			pEnum->Reset();
			pFilter->Release();
		}
		pEnum->Release();
	};
	SAFE_RELEASE(g_pGraph);

	//clean up the rest here
	SAFE_RELEASE(pD3D);
	SAFE_RELEASE(g_pd3dDevice);

	//the following line is actually not needed here 
	//because the custom renderer created and destroyed this texture
	SAFE_RELEASE(g_lpTexture);

	SAFE_RELEASE(g_lpVertexBuffer);
	SAFE_RELEASE(g_lpEffect);

	return 0;
}
#endif // JNK
