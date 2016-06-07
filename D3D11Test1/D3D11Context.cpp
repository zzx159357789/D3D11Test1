#include "stdafx.h"
#include "D3D11Context.h"
#include <D3DX11async.h>
const XMVECTORF32 Silver = { 0.75f,0.75f,0.75f,1.0f };
D3D11CONTEXT_PTR CD3D11Context::instance_ = nullptr;
CD3D11Context::CD3D11Context():
	g_device(NULL),
	g_deviceContext(NULL),
	g_swapChain(NULL),
	g_depthStencilView(NULL),
	g_renderTargetView(NULL),
	w_width(0),
	w_height(0),
	exit_(false)
{
}

CD3D11Context::~CD3D11Context()
{
}

D3D11CONTEXT_PTR CD3D11Context::instance()
{
	if ( !instance_  )
	{
		instance_ = D3D11CONTEXT_PTR(new CD3D11Context());
	}
	return instance_;
}

bool CD3D11Context::initialize(HWND hwnd)
{
	if (!hwnd)
	{
		return false;
	}

	CWnd* m_wnd = CWnd::FromHandle(hwnd);
	CRect rc;
	m_wnd->GetClientRect(rc);
	w_width = rc.Width();
	w_height = rc.Height();
	// 1.创建设备上下文
	D3D_FEATURE_LEVEL features[3] = { D3D_FEATURE_LEVEL_11_0,D3D_FEATURE_LEVEL_10_0,D3D_FEATURE_LEVEL_9_3 };
	D3D_FEATURE_LEVEL actualFeature;
	HRESULT hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, 0, 0,features, 3, D3D11_SDK_VERSION, &g_device, &actualFeature, &g_deviceContext);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"创建D3D11设备失败！", L"错误", MB_OK);
		return false;
	}

	// 2.创建4x采样等级
	UINT msaa4dQuality(0);
	g_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &msaa4dQuality);

	// 3.创建交换链

	// 先描述其属性
	//先描述其属性
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferDesc.Width = w_width;					//宽、高
	swapChainDesc.BufferDesc.Height = w_height;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;			//刷新率
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;					//恒定参数，按照这样指明即可
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	//同上
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						//数据格式，一个为RGBA四元色格式
	swapChainDesc.BufferCount = 1;														//后缓冲区个数，1个足够
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;						//Usage，很好理解
	swapChainDesc.Flags = 0;
	swapChainDesc.OutputWindow = hwnd;							//主窗口句柄
	swapChainDesc.SampleDesc.Count = 4;								//多重采样
	swapChainDesc.SampleDesc.Quality = msaa4dQuality - 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;			//交换：绝大多数情况用DISCARD
	swapChainDesc.Windowed = TRUE;									//窗口模式

																	//获取IDXGIFactory以创建交换链
	IDXGIDevice *dxgiDevice(NULL);
	g_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
	IDXGIAdapter *dxgiAdapter(NULL);
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgiAdapter));
	IDXGIFactory *dxgiFactory(NULL);
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&dxgiFactory));
	hr = dxgiFactory->CreateSwapChain(g_device, &swapChainDesc, &g_swapChain);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"创建d3d11交换链失败！", L"错误！", MB_OK);
		return false;
	}

	dxgiFactory->Release();
	dxgiAdapter->Release();
	dxgiDevice->Release();


	// 5.创建视图

	// 需要先获得资源
	ID3D11Texture2D*  backBuffer(NULL);
	g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	hr = g_device->CreateRenderTargetView(backBuffer, NULL, &g_renderTargetView);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"创建RenderTargetView失败!", L"错误",MB_OK);
		return false;
	}
	backBuffer->Release();

	// 6.创建深度缓冲区视图

	// 要先创建对应缓冲区
	// 创建缓冲区Texture2D,要先给出描述
	D3D11_TEXTURE2D_DESC desc = { 0 };
	desc.Width = w_width;
	desc.Height = w_height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc.Count = 4;
	desc.SampleDesc.Quality = msaa4dQuality - 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	ID3D11Texture2D* depthStencilBuffer(NULL);
	hr = g_device->CreateTexture2D(&desc, NULL, &depthStencilBuffer);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"创建D3D11深度缓冲区失败！", L"错误",MB_OK);
		return false;
	}

	hr = g_device->CreateDepthStencilView(depthStencilBuffer, NULL, &g_depthStencilView);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"创建DepthStencilView失败！", L"错误", MB_OK);
		return false;
	}

	// 7.视图绑定到管线相应阶段
	g_deviceContext->OMSetRenderTargets(1, &g_renderTargetView, g_depthStencilView);
	depthStencilBuffer->Release();

	// 最后设置视口
	D3D11_VIEWPORT viewPort = { 0 };
	viewPort.Width = static_cast<FLOAT>(w_width);
	viewPort.Height = static_cast<FLOAT>(w_height);
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 0.f;
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	g_deviceContext->RSSetViewports(1, &viewPort);

	pm.LoadContent();

	g_hThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	std::thread temThread(&CD3D11Context::handlerFun,this);
	temThread.detach();

	return true;
}

void CD3D11Context::handlerFun()
{
	while (1)
	{
		if (exit_)
		{
			break;
		}
		render();
	}

	SetEvent(g_hThreadEvent);
}

void CD3D11Context::exitThread()
{
	exit_ = true;
	WaitForSingleObject(g_hThreadEvent, INFINITE); //等待事件被触发
	CloseHandle(g_hThreadEvent);
	pm.unLoadContent();
}

void CD3D11Context::render()
{
	//渲染一个绿色的窗口
	XMVECTORF32 color = { 0.f, 1.f, 0.f, 1.6f };
	g_deviceContext->ClearRenderTargetView(g_renderTargetView, reinterpret_cast<const float*>(&Silver));
	g_deviceContext->ClearDepthStencilView(g_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	//全部的场景绘制工作在这里面进行.....
	pm.render();
	//最后显示
	g_swapChain->Present(0, 0);
}

bool CD3D11Context::CompileD3DShader(WCHAR* filePath, char* entry, char* shaderModel, ID3DBlob** buffer)
{
	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* errorBuffer = NULL;
	HRESULT result;

	result = D3DX11CompileFromFile(filePath, 0, 0, entry, shaderModel, shaderFlags, NULL, NULL, buffer, &errorBuffer, NULL);

	if (FAILED(result))
	{
		if (errorBuffer != 0)
		{
			OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
			errorBuffer->Release();
		}

		return false;
	}

	if (errorBuffer != 0)
		errorBuffer->Release();

	return true;
}