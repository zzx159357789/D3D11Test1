#include "stdafx.h"
#include "PlaneMesh.h"
#include "D3D11Context.h"
#include <xnamath.h>


struct VertexPos
{
	XMFLOAT3	pos;
	XMFLOAT4	color;
};

const XMVECTORF32 White = { 1.0f, 1.0f, 1.0f, 1.0f };
const XMVECTORF32 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
const XMVECTORF32 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
const XMVECTORF32 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
const XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
const XMVECTORF32 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
const XMVECTORF32 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
const XMVECTORF32 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
const XMVECTORF32 Silver = { 0.75f,0.75f,0.75f,1.0f };


CPlaneMesh::CPlaneMesh()
{
}


CPlaneMesh::~CPlaneMesh()
{
}


void CPlaneMesh::LoadContent()
{
	ID3DBlob* vsBuffer = 0;
	bool compileResult = CD3D11Context::instance()->CompileD3DShader(L"BasicDraw.fx","VS_Main","vs_4_0",&vsBuffer);
	if (!compileResult)
	{
		MessageBox(0, L"载入的顶点着色器错误！", L"编译错误", MB_OK);
		return ;
	}

	HRESULT d3dResult;
	d3dResult = CD3D11Context::instance()->getD3D11Device()->CreateVertexShader(vsBuffer->GetBufferPointer()
		, vsBuffer->GetBufferSize(), 0, &vs_);
	if (FAILED(d3dResult))
	{
		if (vsBuffer)
		{
			vsBuffer->Release();
		}

		return;
	}

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
		//顶点
		//语义名	输入槽		数据格式			 语义名索引		偏移		输入类型			此处不用，设0
		{ "POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT,	0,			 0,  D3D11_INPUT_PER_VERTEX_DATA,	0 },
		//颜色
		{ "COLOR",	 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,			 12, D3D11_INPUT_PER_VERTEX_DATA,	0 }
	};


	unsigned int totalLayoutElements = ARRAYSIZE(layoutDesc);
	d3dResult = CD3D11Context::instance()->getD3D11Device()->CreateInputLayout(layoutDesc, totalLayoutElements,
		vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &inputLayout_);

	vsBuffer->Release();

	if (FAILED(d3dResult))
	{
		return;
	}


	ID3DBlob* psBuffer = 0;
	compileResult = CD3D11Context::instance()->CompileD3DShader(L"BasicDraw.fx", "PS_Main", "ps_4_0", &psBuffer);
	if (!compileResult)
	{
		MessageBox(0, L"载入像素着色器错误!", L"编译错误", MB_OK);
		return;
	}

	d3dResult = CD3D11Context::instance()->getD3D11Device()->CreatePixelShader(psBuffer->GetBufferPointer(),
		psBuffer->GetBufferSize(), 0, &ps_);
	psBuffer->Release();
	if (FAILED(d3dResult))
	{
		return;
	}

	VertexPos vertice[]=
	{
		{ XMFLOAT3(-1.f,-1.f,-1.f),reinterpret_cast<const float*>(&Blue) },
		{ XMFLOAT3(-1.f, 1.f,-1.f),reinterpret_cast<const float*>(&Cyan) },
		{ XMFLOAT3(1.f, 1.f,-1.f),reinterpret_cast<const float*>(&Red) },
		{ XMFLOAT3(1.f,-1.f,-1.f),reinterpret_cast<const float*>(&Yellow) },
		{ XMFLOAT3(-1.f,-1.f, 1.f),reinterpret_cast<const float*>(&Green) },
		{ XMFLOAT3(-1.f, 1.f, 1.f),reinterpret_cast<const float*>(&Silver) },
		{ XMFLOAT3(1.f, 1.f, 1.f),reinterpret_cast<const float*>(&Black) },
		{ XMFLOAT3(1.f,-1.f, 1.f),reinterpret_cast<const float*>(&Magenta) }
	};

	//创建顶点缓存
	//首先创建描述
	D3D11_BUFFER_DESC vertexDesc = { 0 };
	vertexDesc.ByteWidth = 8 * sizeof(vertexDesc);
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = vertice;

	d3dResult = CD3D11Context::instance()->getD3D11Device()->CreateBuffer(&vertexDesc, &resourceData, &vertexBuffer_);
	if (FAILED(d3dResult))
	{
		return;
	}

	//创建索引缓存
	//首先给出描述
	D3D11_BUFFER_DESC ibDesc = { 0 };
	ibDesc.ByteWidth = 36 * sizeof(UINT);
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	//然后给出数据
	UINT indices[36] =
	{
		0,1,2, 0,2,3,
		4,5,1, 4,1,0,
		7,6,5, 7,5,4,
		3,2,6, 3,6,7,
		1,5,6, 1,6,2,
		4,0,3, 4,3,7
	};
	D3D11_SUBRESOURCE_DATA ibData = { 0 };
	ibData.pSysMem = indices;
	//根据描述和数据创建索引缓存
	d3dResult = CD3D11Context::instance()->getD3D11Device()->CreateBuffer(&ibDesc, &ibData, &g_IB);
	if (FAILED(d3dResult))
	{
		MessageBox(NULL, L"CreateIndexBuffer错误!", L"错误", MB_OK);
		return ;
	}
}

void CPlaneMesh::render()
{
	if ( !CD3D11Context::instance()->getD3D11Device() || !vs_ || !ps_)
	{
		return;
	}

	unsigned int stride = sizeof(VertexPos);
	unsigned int offset = 0;

	CD3D11Context::instance()->getD3D11DeviceContext()->IASetInputLayout(inputLayout_);
	CD3D11Context::instance()->getD3D11DeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
	//指定索引缓存
	CD3D11Context::instance()->getD3D11DeviceContext()->IASetIndexBuffer(g_IB, DXGI_FORMAT_R32_UINT, 0);

	CD3D11Context::instance()->getD3D11DeviceContext()->VSSetShader(vs_, 0, 0);
	CD3D11Context::instance()->getD3D11DeviceContext()->PSSetShader(ps_, 0, 0);
	CD3D11Context::instance()->getD3D11DeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CD3D11Context::instance()->getD3D11DeviceContext()->DrawIndexed(36, 0,0);

}

void CPlaneMesh::unLoadContent()
{
	if (vs_) vs_->Release();
	if (ps_) ps_->Release();
	if (inputLayout_) inputLayout_->Release();
	if (vertexBuffer_) vertexBuffer_->Release();
	
	vs_ = 0;
	ps_ = 0;
	inputLayout_ = 0;
	vertexBuffer_ = 0;
}


