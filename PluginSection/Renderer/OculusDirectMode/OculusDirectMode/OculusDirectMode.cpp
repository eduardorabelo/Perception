/********************************************************************
Vireio Perception: Open-Source Stereoscopic 3D Driver
Copyright (C) 2012 Andres Hernandez

Vireio Oculus Direct Mode - Oculus Rift DirectMode Node Plugin
Copyright (C) 2015 Denis Reischl

File <OculusDirectMode.cpp> and
Class <OculusDirectMode> :
Copyright (C) 2015 Denis Reischl

The stub class <AQU_Nodus> is the only public class from the Aquilinus 
repository and permitted to be used for open source plugins of any kind. 
Read the Aquilinus documentation for further information.

Vireio Perception Version History:
v1.0.0 2012 by Andres Hernandez
v1.0.X 2013 by John Hicks, Neil Schneider
v1.1.x 2013 by Primary Coding Author: Chris Drain
Team Support: John Hicks, Phil Larkson, Neil Schneider
v2.0.x 2013 by Denis Reischl, Neil Schneider, Joshua Brown
v2.0.4 onwards 2014 by Grant Bagwell, Simon Brown and Neil Schneider

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#include"OculusDirectMode.h"

#define INTERFACE_IDIRECT3DDEVICE9 8
#define INTERFACE_IDIRECT3DSWAPCHAIN9 15

#define INTERFACE_IDIRECT3DDEVICE9           8
#define INTERFACE_IDIRECT3DSWAPCHAIN9       15
#define	METHOD_IDIRECT3DDEVICE9_PRESENT     17
#define	METHOD_IDIRECT3DDEVICE9_ENDSCENE    42
#define	METHOD_IDIRECT3DSWAPCHAIN9_PRESENT   3

/**
* Constructor.
***/
OculusDirectMode::OculusDirectMode() : AQU_Nodus(),
	m_pcDeviceTemporary(nullptr),
	m_pcContextTemporary(nullptr),
	m_psMainDepthBuffer(nullptr),
	m_psUniformBufferGen(nullptr),
	m_pcBackBuffer(nullptr),
	m_pcBackBufferRT(nullptr),
	m_bInit(false),
	m_pcMirrorTexture(nullptr)
{	
}

/**
* Destructor.
***/
OculusDirectMode::~OculusDirectMode()
{
	if (m_pcContextTemporary) m_pcContextTemporary->Release();
	if (m_pcDeviceTemporary) m_pcDeviceTemporary->Release();
	if (m_psMainDepthBuffer) delete m_psMainDepthBuffer;
	if (m_psUniformBufferGen) delete m_psUniformBufferGen;
	if (m_pcBackBuffer) m_pcBackBuffer->Release();
	if (m_pcBackBufferRT) m_pcBackBufferRT->Release();
}

/**
* Return the name of the  Oculus Renderer node.
***/
const char* OculusDirectMode::GetNodeType() 
{
	return "Oculus Direct Mode"; 
}

/**
* Returns a global unique identifier for the Oculus Renderer node.
***/
UINT OculusDirectMode::GetNodeTypeId()
{
#define DEVELOPER_IDENTIFIER 2006
#define MY_PLUGIN_IDENTIFIER 259
	return ((DEVELOPER_IDENTIFIER << 16) + MY_PLUGIN_IDENTIFIER);
}

/**
* Returns the name of the category for the Oculus Renderer node.
***/
LPWSTR OculusDirectMode::GetCategory()
{
	return L"Renderer";
}

/**
* Returns a logo to be used for the Oculus Renderer node.
***/
HBITMAP OculusDirectMode::GetLogo() 
{ 
	HMODULE hModule = GetModuleHandle(L"OculusDirectMode.dll");
	HBITMAP hBitmap = LoadBitmap(hModule, MAKEINTRESOURCE(IMG_LOGO01));
	return hBitmap;
}

/** 
* Returns the updated control for the Oculus Renderer node.
* Allways return >nullptr< if there is no update for the control !!
***/
HBITMAP OculusDirectMode::GetControl()
{
	//if (!m_hBitmapControl)
	//{
	// TODO : Design the Oculus direct mode node, both Logo and Control
	//}

	return nullptr;
}

/**
* Provides the name of the requested decommander.
***/
LPWSTR OculusDirectMode::GetDecommanderName(DWORD dwDecommanderIndex)
{
	switch ((ORN_Decommanders)dwDecommanderIndex)
	{
	case ORN_Decommanders::LeftTexture:
		return L"Left Texture";
	case ORN_Decommanders::RightTexture:
		return L"Right Texture";
	}

	return L"";
}

/**
* Returns the plug type for the requested decommander.
***/
DWORD OculusDirectMode::GetDecommanderType(DWORD dwDecommanderIndex) 
{
	//switch ((ORN_Decommanders)dwDecommanderIndex)
	//{
	//	/*case ORN_Decommanders::LeftTexture:
	//	return PNT_IDIRECT3DTEXTURE9_PLUG_TYPE;
	//	case ORN_Decommanders::RightTexture:
	//	return PNT_IDIRECT3DTEXTURE9_PLUG_TYPE;*/
	//}

	return 0;
}

/**
* Sets the input pointer for the requested decommander.
***/
void OculusDirectMode::SetInputPointer(DWORD dwDecommanderIndex, void* pData)
{
}

/**
* Oculus Direct Mode supports D3D 9 - 11 Present() calls.
***/
bool OculusDirectMode::SupportsD3DMethod(int nD3DVersion, int nD3DInterface, int nD3DMethod)  
{ 
	/*if ((nD3DVersion >= (int)AQU_DirectXVersion::DirectX_9_0) &&
	(nD3DVersion <= (int)AQU_DirectXVersion::DirectX_9_29))
	{
	if (((nD3DInterface == INTERFACE_IDIRECT3DDEVICE9) &&
	(nD3DMethod == METHOD_IDIRECT3DDEVICE9_PRESENT)) || 
	((nD3DInterface == INTERFACE_IDIRECT3DDEVICE9) &&
	(nD3DMethod == METHOD_IDIRECT3DDEVICE9_ENDSCENE)) ||
	((nD3DInterface == INTERFACE_IDIRECT3DSWAPCHAIN9) &&
	(nD3DMethod == METHOD_IDIRECT3DSWAPCHAIN9_PRESENT)))
	return true;
	}*/
	return true; 
}

/**
* Render the Oculus Rift View.
***/
void* OculusDirectMode::Provoke(void* pThis, int eD3D, int eD3DInterface, int eD3DMethod, DWORD dwNumberConnected, int& nProvokerIndex)	
{
	// cast swapchain
	IDXGISwapChain* pcSwapChain = (IDXGISwapChain*)pThis;
	if (!pcSwapChain) 
	{
		OutputDebugString(L"Oculus Direct Mode Node : No swapchain !");
		return nullptr;
	}

	if (!m_bInit)
	{
		// get device
		ID3D11Device* pcDevice = nullptr;
		pcSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pcDevice);
		if (!pcDevice)
		{
			OutputDebugString(L"HelloWorld Node : No d3d 11 device !");
			return nullptr;
		}
		// get context
		ID3D11DeviceContext* pcContext = nullptr;
		pcDevice->GetImmediateContext(&pcContext);
		if (!pcContext)
		{
			OutputDebugString(L"HelloWorld Node : No device context !");
			return nullptr;
		}

		// get render target view
		ID3D11RenderTargetView* ppRenderTargetView[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		ID3D11DepthStencilView* pDepthStencilView = nullptr;
		pcContext->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, ppRenderTargetView, &pDepthStencilView);
		UINT dwNumViewports = 1;
		D3D11_VIEWPORT sViewport[16];
		pcContext->RSGetViewports(&dwNumViewports, sViewport);

		// create temporary (OVR) device
		IDXGIFactory * DXGIFactory;
		IDXGIAdapter * Adapter;
		if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)(&DXGIFactory))))
			return(false);
		if (FAILED(DXGIFactory->EnumAdapters(0, &Adapter)))
			return(false);
		if (FAILED(D3D11CreateDevice(Adapter, Adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
			NULL, 0, NULL, 0, D3D11_SDK_VERSION, &m_pcDeviceTemporary, NULL, &m_pcContextTemporary)))
			return(false);

		// Create backbuffer
		if (FAILED(pcSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_pcBackBuffer))) return(false);
		if (FAILED(pcDevice->CreateRenderTargetView(m_pcBackBuffer, NULL, &m_pcBackBufferRT)))         return(false);

		// Main depth buffer
		OVR::Sizei sz;
		sz.w = (int)sViewport[0].Width;
		sz.h = (int)sViewport[0].Height;
		m_psMainDepthBuffer = new DepthBuffer(pcDevice, sz);
		pcContext->OMSetRenderTargets(1, &m_pcBackBufferRT, m_psMainDepthBuffer->TexDsv);

		// Buffer for shader constants
		// #define                  UNIFORM_DATA_SIZE 2000  // Fixed size buffer for shader constants, before copied into buffer
		m_psUniformBufferGen = new DataBuffer(pcDevice, D3D11_BIND_CONSTANT_BUFFER, NULL, 2000 /*UNIFORM_DATA_SIZE*/);
		pcContext->VSSetConstantBuffers(0, 1, &m_psUniformBufferGen->D3DBuffer);

		// Set a standard blend state, ie transparency from alpha
		D3D11_BLEND_DESC bm;
		memset(&bm, 0, sizeof(bm));
		bm.RenderTarget[0].BlendEnable = TRUE;
		bm.RenderTarget[0].BlendOp = bm.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bm.RenderTarget[0].SrcBlend = bm.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		bm.RenderTarget[0].DestBlend = bm.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		bm.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		ID3D11BlendState * BlendState;
		m_pcDeviceTemporary->CreateBlendState(&bm, &BlendState);
		m_pcContextTemporary->OMSetBlendState(BlendState, NULL, 0xffffffff);

		// Set max frame latency to 1
		IDXGIDevice1* DXGIDevice1 = NULL;
		HRESULT hr = m_pcDeviceTemporary->QueryInterface(__uuidof(IDXGIDevice1), (void**)&DXGIDevice1);
		if (FAILED(hr) | (DXGIDevice1 == NULL)) return(false);
		DXGIDevice1->SetMaximumFrameLatency(1);
		DXGIDevice1->Release();

		// Initialize LibOVR, and the Rift... then create hmd handle
		ovrResult result = ovr_Initialize(nullptr);
		if (!OVR_SUCCESS(result)) 
		{
			OutputDebugString(L"Failed to initialize libOVR.");
			return nullptr;
		}
		ovrResult actualHMD = ovrHmd_Create(0, &m_hHMD);
		if (!OVR_SUCCESS(actualHMD)) result = ovrHmd_CreateDebug(ovrHmd_DK2, &m_hHMD); // Use debug one, if no genuine Rift available
		if (!OVR_SUCCESS(result)) OutputDebugString(L"Oculus Rift not detected.");
		if (m_hHMD->ProductName[0] != '\0') OutputDebugString(L"Rift detected, display not enabled.");

		ovrHmd_SetEnabledCaps(m_hHMD, ovrHmdCap_LowPersistence|ovrHmdCap_DynamicPrediction);

		// Make the eye render buffers (caution if actual size < requested due to HW limits). 
		for (int eye = 0; eye < 2; eye++)
		{
			OVR::Sizei idealSize = ovrHmd_GetFovTextureSize(m_hHMD, (ovrEyeType)eye, m_hHMD->DefaultEyeFov[eye], 1.0f);

			m_psEyeRenderTexture[eye]      = new OculusTexture(m_pcDeviceTemporary, m_hHMD, idealSize);
			m_psEyeDepthBuffer[eye]        = new DepthBuffer(m_pcDeviceTemporary, idealSize);
			m_psEyeRenderViewport[eye].Pos  = OVR::Vector2i(0, 0);
			m_psEyeRenderViewport[eye].Size = idealSize;
		}

		// Create a mirror to see on the monitor.
		D3D11_TEXTURE2D_DESC td = { };
		td.ArraySize        = 1;
		td.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;
		td.Width            = (UINT)sViewport[0].Width;
		td.Height           = (UINT)sViewport[0].Height;
		td.Usage            = D3D11_USAGE_DEFAULT;
		td.SampleDesc.Count = 1;
		td.MipLevels        = 1;
		ovrHmd_CreateMirrorTextureD3D11(m_hHMD, m_pcDeviceTemporary, &td, &m_pcMirrorTexture);

		// Setup VR components, filling out description
		m_psEyeRenderDesc[0] = ovrHmd_GetRenderDesc(m_hHMD, ovrEye_Left, m_hHMD->DefaultEyeFov[0]);
		m_psEyeRenderDesc[1] = ovrHmd_GetRenderDesc(m_hHMD, ovrEye_Right, m_hHMD->DefaultEyeFov[1]);

		// finish up
		for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		{
			if (ppRenderTargetView[i]) { ppRenderTargetView[i]->Release(); ppRenderTargetView[i] = nullptr; }
		}
		if (pDepthStencilView) { pDepthStencilView->Release(); pDepthStencilView = nullptr; }

		// release d3d11 device + context... 
		pcContext->Release();
		pcDevice->Release();

		m_bInit = true;
	}
	else
	{
		// basic code here taken from OculusRoomTiny example
		ovrVector3f      HmdToEyeViewOffset[2] = { m_psEyeRenderDesc[0].HmdToEyeViewOffset,
			m_psEyeRenderDesc[1].HmdToEyeViewOffset };
		ovrPosef         EyeRenderPose[2];
		ovrFrameTiming   ftiming  = ovrHmd_GetFrameTiming(m_hHMD, 0);
		ovrTrackingState hmdState = ovrHmd_GetTrackingState(m_hHMD, ftiming.DisplayMidpointSeconds);
		ovr_CalcEyePoses(hmdState.HeadPose.ThePose, HmdToEyeViewOffset, EyeRenderPose);

		// Initialize our single full screen Fov layer.
		ovrLayerEyeFov ld;
		ld.Header.Type  = ovrLayerType_EyeFov;
		ld.Header.Flags = 0;

		for (int eye = 0; eye < 2; eye++)
		{
			ld.ColorTexture[eye] = m_psEyeRenderTexture[eye]->TextureSet;
			ld.Viewport[eye]     = m_psEyeRenderViewport[eye];
			ld.Fov[eye]          = m_hHMD->DefaultEyeFov[eye];
			ld.RenderPose[eye]   = EyeRenderPose[eye];
		}

		// Set up positional data.
		ovrViewScaleDesc viewScaleDesc;
		viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
		viewScaleDesc.HmdToEyeViewOffset[0] = HmdToEyeViewOffset[0];
		viewScaleDesc.HmdToEyeViewOffset[1] = HmdToEyeViewOffset[1];

		ovrLayerHeader* layers = &ld.Header;
		ovrResult result = ovrHmd_SubmitFrame(m_hHMD, 0, &viewScaleDesc, &layers, 1);
	}
	return nullptr;
}