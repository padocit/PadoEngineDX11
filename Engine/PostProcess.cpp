#include "PostProcess.h"
#include "D3D11Common.h"

void PostProcess::Initialize(
    ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
    const std::vector<ComPtr<ID3D11ShaderResourceView>> &resources,
    const std::vector<ComPtr<ID3D11RenderTargetView>> &targets, const int width,
    const int height, const int bloomLevels)
{

    MeshData meshData = GeometryGenerator::MakeSquare();

    mesh = std::make_shared<Mesh>();
    D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
                                   mesh->vertexBuffer);
    mesh->indexCount = UINT(meshData.indices.size());
    D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                                  mesh->indexBuffer);

    // Bloom Down/Up
    bloomSRVs.resize(bloomLevels);
    bloomRTVs.resize(bloomLevels);
    for (int i = 0; i < bloomLevels; i++)
    {
        int div = int(pow(2, i));
        CreateBuffer(device, context, width / div, height / div, bloomSRVs[i],
                     bloomRTVs[i]);
    }

    bloomDownFilters.resize(bloomLevels - 1);
    for (int i = 0; i < bloomLevels - 1; i++)
    {
        int div = int(pow(2, i + 1));
        bloomDownFilters[i].Initialize(device, context, Graphics::bloomDownPS,
                                         width / div, height / div);
        if (i == 0)
        {
            bloomDownFilters[i].SetShaderResources({resources[0]});
        }
        else
        {
            bloomDownFilters[i].SetShaderResources({bloomSRVs[i]});
        }

        bloomDownFilters[i].SetRenderTargets({bloomRTVs[i + 1]});
    }

    bloomUpFilters.resize(bloomLevels - 1);
    for (int i = 0; i < bloomLevels - 1; i++)
    {
        int level = bloomLevels - 2 - i;
        int div = int(pow(2, level));
        bloomUpFilters[i].Initialize(device, context, Graphics::bloomUpPS,
                                       width / div, height / div);
        bloomUpFilters[i].SetShaderResources({bloomSRVs[level + 1]});
        bloomUpFilters[i].SetRenderTargets({bloomRTVs[level]});
    }
    
    // Combine + ToneMapping (최적화 - 쉐이더 호출 줄이기)
    combineFilter.Initialize(device, context, Graphics::combinePS, width,
                               height);
    combineFilter.SetShaderResources(
        {resources[0], bloomSRVs[0],
         resources[1]}); // resource[1]은 모션 블러를 위한 이전 프레임 결과
    combineFilter.SetRenderTargets(targets);
    combineFilter.constData.strength = 0.0f; // Bloom strength
    combineFilter.constData.option1 = 1.0f;  // Exposure로 사용
    combineFilter.constData.option2 = 2.2f;  // Gamma로 사용

    // 주의: float render target에서는 Gamma correction 하지 않음 (gamma = 1.0)

    combineFilter.UpdateConstantBuffers(context);
}

void PostProcess::Render(ComPtr<ID3D11DeviceContext> &context)
{

    context->PSSetSamplers(0, 1, Graphics::linearClampSS.GetAddressOf());

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                &stride, &offset);
    context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT,
                              0);

    // 블룸이 필요한 경우에만 계산
    if (combineFilter.constData.strength > 0.0f)
    {
        for (int i = 0; i < bloomDownFilters.size(); i++)
        {
            RenderImageFilter(context, bloomDownFilters[i]);
        }

        for (int i = 0; i < bloomUpFilters.size(); i++)
        {
            RenderImageFilter(context, bloomUpFilters[i]);
        }
    }

    RenderImageFilter(context, combineFilter);
}

void PostProcess::RenderImageFilter(ComPtr<ID3D11DeviceContext> &context,
                                    const ImageFilter &imageFilter)
{
    imageFilter.Render(context);
    context->DrawIndexed(mesh->indexCount, 0, 0); // mesh->indexCount 필요하므로 여기서 Draw call
}

void PostProcess::CreateBuffer(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context, int width,
                               int height,
                               ComPtr<ID3D11ShaderResourceView> &srv,
                               ComPtr<ID3D11RenderTargetView> &rtv)
{

    ComPtr<ID3D11Texture2D> texture;

    D3D11_TEXTURE2D_DESC txtDesc;
    ZeroMemory(&txtDesc, sizeof(txtDesc));
    txtDesc.Width = width;
    txtDesc.Height = height;
    txtDesc.MipLevels = txtDesc.ArraySize = 1;
    txtDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; //  이미지 처리용도
    txtDesc.SampleDesc.Count = 1;
    txtDesc.Usage = D3D11_USAGE_DEFAULT;
    txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    txtDesc.MiscFlags = 0;
    txtDesc.CPUAccessFlags = 0;

    ThrowIfFailed(
        device->CreateTexture2D(&txtDesc, NULL, texture.GetAddressOf()));
    ThrowIfFailed(device->CreateRenderTargetView(texture.Get(), NULL,
                                                 rtv.GetAddressOf()));
    ThrowIfFailed(device->CreateShaderResourceView(texture.Get(), NULL,
                                                   srv.GetAddressOf()));
}
