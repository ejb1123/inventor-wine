// Assemble a self-authored LLVM .ll module using the public DXC assembler API.
#include <dxc/dxcapi.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>
template <typename T> struct Release { void operator()(T *p) const { if (p) p->Release(); } };
template <typename T> using Com = std::unique_ptr<T, Release<T>>;
int main(int argc, char **argv)
{
    if (argc != 3) { std::cerr << "Usage: assemble-dxil input.ll output.dxil\n"; return 2; }
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) return 2;
    std::vector<char> text((std::istreambuf_iterator<char>(input)), {});
    if (text.empty() || text.size() > UINT32_MAX) return 2;
    IDxcLibrary *library_raw = nullptr;
    if (FAILED(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library_raw)))) return 1;
    Com<IDxcLibrary> library(library_raw);
    IDxcBlobEncoding *blob_raw = nullptr;
    if (FAILED(library->CreateBlobWithEncodingFromPinned(text.data(), UINT32(text.size()), DXC_CP_UTF8, &blob_raw))) return 1;
    Com<IDxcBlobEncoding> blob(blob_raw);
    IDxcAssembler *assembler_raw = nullptr;
    if (FAILED(DxcCreateInstance(CLSID_DxcAssembler, IID_PPV_ARGS(&assembler_raw)))) return 1;
    Com<IDxcAssembler> assembler(assembler_raw);
    IDxcOperationResult *result_raw = nullptr;
    if (FAILED(assembler->AssembleToContainer(blob.get(), &result_raw))) return 1;
    Com<IDxcOperationResult> result(result_raw);
    HRESULT status;
    if (FAILED(result->GetStatus(&status))) return 1;
    if (FAILED(status)) {
        IDxcBlobEncoding *errors_raw = nullptr;
        result->GetErrorBuffer(&errors_raw);
        Com<IDxcBlobEncoding> errors(errors_raw);
        if (errors) std::cerr.write(static_cast<const char *>(errors->GetBufferPointer()), errors->GetBufferSize());
        return 1;
    }
    IDxcBlob *output_raw = nullptr;
    if (FAILED(result->GetResult(&output_raw))) return 1;
    Com<IDxcBlob> output(output_raw);
    std::ofstream file(argv[2], std::ios::binary);
    file.write(static_cast<const char *>(output->GetBufferPointer()), output->GetBufferSize());
    return file ? 0 : 1;
}
