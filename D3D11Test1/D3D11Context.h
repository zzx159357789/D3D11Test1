#ifndef  D3D11CONTEXT_H
#define D3D11CONTEXT_H

#ifndef _WINDOWS
#include <Windows.h>
#endif


//#include <d3dx11.h>
#include <d3d11.h>
#include <thread>
#include<memory>
#include <string>
#include <xnamath.h>

class CD3D11Context;
typedef std::shared_ptr< CD3D11Context >  D3D11CONTEXT_PTR;
class CD3D11Context
{
public:
	static D3D11CONTEXT_PTR instance();
	bool  initialize( HWND hwnd );
	void   exitThread();
	~CD3D11Context();
private:
	CD3D11Context();
	void handlerFun();
	void render();
private:
		static D3D11CONTEXT_PTR	instance_;
		int											w_width;
		int											w_height;
		bool										exit_;
		HANDLE								g_hThreadEvent;
		// D3D相关全局变量
		ID3D11Device						*g_device;
		ID3D11DeviceContext			*g_deviceContext;
		IDXGISwapChain					*g_swapChain;

		ID3D11DepthStencilView		*g_depthStencilView;
		ID3D11RenderTargetView		*g_renderTargetView;
};
#endif // ! D3D11CONTEXT_H
