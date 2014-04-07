////////////////////////////////////////////////////////////////////////////////
// Filename: Tessellationshaderclass.h
// based on code from http://www.rastertek.com/tutdx11.html April 2012
////////////////////////////////////////////////////////////////////////////////
#ifndef _ComputeSHADERCLASS_H_
#define _ComputeSHADERCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
#include "Api.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: ComputeShaderClass
////////////////////////////////////////////////////////////////////////////////
class ComputeShader
{
private:
	//need to use the Particle struct here

public:
	ComputeShader(int num);
	ComputeShader(const ComputeShader&);
	~ComputeShader();

	bool initialize(ID3D11Device*, HWND);
	void shutdown();
	bool render(ID3D11DeviceContext*, ID3D11ShaderResourceView*,  float);

private:
	bool initializeShader(ID3D11Device*, HWND, WCHAR*);
	void shutdownShader();
	void outputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool setShaderParameters(ID3D11DeviceContext*, ID3D11ShaderResourceView*, float);
	void renderShader(ID3D11DeviceContext*, int);

private:
	ID3D11ComputeShader* m_computeShader;
	ID3D11InputLayout* m_layout;//?
	ID3D11Buffer* m_inputBuffer;
	ID3D11Buffer* m_outputBuffer;
	ID3D11ShaderResourceView *m_InputSRV;
	ID3D11UnorderedAccessView *m_OutputUAV;
	
	int m_numParticles;
};

#endif