#include "D3D11Utils.h"

void CheckResult(HRESULT hr, ID3DBlob *errorBlob)
{
    if (FAILED(hr)) {
        // ������ ���� ���
        if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0) {
            std::cout << "File not found." << std::endl;
        }

        // ���� �޽����� ������ ���
        if (errorBlob) {
            std::cout << "Shader compile error\n"
                      << (char *)errorBlob->GetBufferPointer() << std::endl;
        }
    }
}
