#ifndef PLANE_H
#define PLANE_H
#include <d3d11.h>

class CPlaneMesh
{
public:
	CPlaneMesh();
	~CPlaneMesh();
	void LoadContent();
	void unLoadContent();
	void render();
private:
	ID3D11VertexShader*  vs_;
	ID3D11PixelShader*     ps_;
	ID3D11InputLayout*	  inputLayout_;
	ID3D11Buffer*			  vertexBuffer_;
	ID3D11Buffer	*			  g_IB;

};
#endif