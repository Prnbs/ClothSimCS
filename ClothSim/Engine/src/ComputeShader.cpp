////////////////////////////////////////////////////////////////////////////////
// Filename: ComputeShaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ComputeShader.h"
#include "Solver.h"

using namespace pjs;

ComputeShader::ComputeShader(int num)
{
	m_computeShader = 0;
	m_layout = 0;
	m_inputBuffer = 0;
	m_outputBuffer = 0;
	m_numParticles = num;
}


ComputeShader::ComputeShader(const ComputeShader& other)
{
}


ComputeShader::~ComputeShader()
{
}


bool ComputeShader::initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	// Initialize the vertex and pixel shaders.
	result = initializeShader(device, hwnd, 
		L"../ClothSim/Engine/shaders/verlet.hlsl");
	if(!result)
	{
		return false;
	}

	return true;
}


void ComputeShader::shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	//ComputeShader(m_numParticles );
	return;
}


bool ComputeShader::render(
	ID3D11DeviceContext* deviceContext,
	ID3D11ShaderResourceView* Tessellation, float _tesssellationAmount
	)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = setShaderParameters(
		deviceContext, Tessellation,  _tesssellationAmount);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	renderShader(deviceContext);

	return true;
}


bool ComputeShader::initializeShader(ID3D11Device* device, HWND hwnd, WCHAR* csFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* computeShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
	D3D11_BUFFER_DESC inputBufferDesc;
	D3D11_BUFFER_DESC outputBufferDesc;

	// Initialize the pointers this function will use to null.
	errorMessage = nullptr;
	computeShaderBuffer = nullptr;

    // Compile the vertex shader code.
	result = D3DX11CompileFromFile(csFilename, NULL, NULL, "VerletComputeShader", "cs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
									&computeShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			outputShaderErrorMessage(errorMessage, hwnd, csFilename);
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, csFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    // Create the compute shader from the buffer.
	result = device->CreateComputeShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(), NULL, &m_computeShader);
	if(FAILED(result))
	{
		return false;
	}

	// Create the compute input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;
	
	polygonLayout[1].SemanticName = "NORMAL";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;
	
	polygonLayout[2].SemanticName = "TEXCOORD";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), 
		                               &m_layout);
	if(FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	computeShaderBuffer->Release();
	computeShaderBuffer = 0;

	// Setup the description of the input structure to the Compute Shader
    inputBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	inputBufferDesc.ByteWidth = sizeof(pjs::Particle) * m_numParticles;
	inputBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	inputBufferDesc.CPUAccessFlags = 0;
	inputBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	inputBufferDesc.StructureByteStride = sizeof(pjs::Particle);

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&inputBufferDesc, NULL, &m_inputBuffer);
	if(FAILED(result))
	{
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.NumElements = numElements;

	result = device->CreateShaderResourceView(m_inputBuffer, &srvDesc, &m_InputSRV);
	if (FAILED(result))
	{
		return false;
	}

	outputBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	outputBufferDesc.ByteWidth = sizeof(pjs::Particle) * m_numParticles;
	outputBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	outputBufferDesc.CPUAccessFlags = 0;
	outputBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	outputBufferDesc.StructureByteStride = sizeof(pjs::Particle);

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&outputBufferDesc, NULL, &m_outputBuffer);
	if(FAILED(result))
	{
		return false;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.NumElements = m_numParticles;

	result = device->CreateUnorderedAccessView(m_outputBuffer, &uavDesc, &m_OutputUAV);

	return true;
}


void ComputeShader::shutdownShader()
{
	// Release the input buffer.
	if(m_inputBuffer)
	{
		m_inputBuffer->Release();
		m_inputBuffer = 0;
	}

	// Release the output buffer.
	if(m_outputBuffer)
	{
		m_outputBuffer->Release();
		m_outputBuffer = 0;
	}

	// Release the layout.
	if(m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the vertex shader.
	if(m_computeShader)
	{
		m_computeShader->Release();
		m_computeShader = 0;
	}

	return;
}


void ComputeShader::outputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	std::ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}


bool ComputeShader::setShaderParameters(
	ID3D11DeviceContext* deviceContext, 
	ID3D11ShaderResourceView* Tessellation, float _tessellationAmount
	)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matData;
	TessellationBufferType* tessData;
	LightBufferType* lightData;
	unsigned int bufferNumber;


	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Set shader Tessellation resource in the pixel shader.
	deviceContext->CSSetShaderResources(0, 1, &Tessellation);

	// Lock the light constant buffer so it can be written to.
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	lightData = (LightBufferType*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	lightData->ambientColor = _ambientColor;
	lightData->diffuseColor = _lightColor;
	lightData->lightDir = _lightDirection;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_lightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

	// Lock the tessellation constant buffer so it can be written to.
	result = deviceContext->Map(m_tessellationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the tessellation constant buffer.
	tessData = (TessellationBufferType*)mappedResource.pData;

	// Copy the tessellation data into the constant buffer.
	tessData->tessellationAmount = _tessellationAmount;
	tessData->padding = pjs::Vec3(0.0f, 0.0f, 0.0f);

	// Unlock the tessellation constant buffer.
	deviceContext->Unmap(m_tessellationBuffer, 0);

	// Set the position of the tessellation constant buffer in the hull shader.
	bufferNumber = 0;

	// Now set the tessellation constant buffer in the hull shader with the updated values.
	deviceContext->HSSetConstantBuffers(bufferNumber, 1, &m_tessellationBuffer);


	return true;
}


void ComputeShader::renderShader(ID3D11DeviceContext* deviceContext)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

    // Set the compute shaders that will be used
	deviceContext->CSSetShader(m_computeShader, NULL, 0);

	// Render the triangle.
	deviceContext->Dispatch(1,1,1);

	return;
}