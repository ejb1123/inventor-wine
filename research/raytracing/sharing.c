#define COBJMACROS
#include <windows.h>
#include <initguid.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#define printf(...) do { char buf[1024]; DWORD n; wsprintfA(buf,__VA_ARGS__); WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),buf,lstrlenA(buf),&n,0); } while(0)
#define CHECK(call) do { HRESULT hr=(call); printf("%s: 0x%08lx\n",#call,(unsigned long)hr); if(FAILED(hr)) return 1; } while(0)
static int probe(void) {
 HRESULT (WINAPI *create)(IUnknown*,D3D_FEATURE_LEVEL,REFIID,void**);
 HRESULT (WINAPI *factoryfn)(REFIID,void**);
 HMODULE d=LoadLibraryA("d3d12.dll"), g=LoadLibraryA("dxgi.dll");
 if(!d||!g) {printf("DLL load error %lu\n",GetLastError());return 1;}
 create=(void*)GetProcAddress(d,"D3D12CreateDevice");factoryfn=(void*)GetProcAddress(g,"CreateDXGIFactory1");
 IDXGIFactory1 *f=NULL; IDXGIAdapter1 *a=NULL; ID3D12Device *dev=NULL;ID3D12CommandQueue *q=NULL;ID3D12Fence *fence=NULL;
 CHECK(factoryfn(&IID_IDXGIFactory1,(void**)&f));
 CHECK(IDXGIFactory1_EnumAdapters1(f,0,&a));
 DXGI_ADAPTER_DESC1 desc;CHECK(IDXGIAdapter1_GetDesc1(a,&desc));printf("Adapter vendor=%04x device=%04x\n",desc.VendorId,desc.DeviceId);
 CHECK(create((IUnknown*)a,D3D_FEATURE_LEVEL_12_0,&IID_ID3D12Device,(void**)&dev));
 D3D12_FEATURE_DATA_D3D12_OPTIONS5 opts={0};
 CHECK(ID3D12Device_CheckFeatureSupport(dev,D3D12_FEATURE_D3D12_OPTIONS5,&opts,sizeof(opts)));
 printf("RaytracingTier=%u (0=unsupported,10=1.0,11=1.1)\n",opts.RaytracingTier);
 D3D12_HEAP_PROPERTIES heap={0};heap.Type=D3D12_HEAP_TYPE_DEFAULT;heap.CreationNodeMask=1;heap.VisibleNodeMask=1;
 D3D12_RESOURCE_DESC desc2={0};desc2.Dimension=D3D12_RESOURCE_DIMENSION_TEXTURE2D;desc2.Width=64;desc2.Height=64;desc2.DepthOrArraySize=1;desc2.MipLevels=1;desc2.Format=DXGI_FORMAT_R16G16B16A16_FLOAT;desc2.SampleDesc.Count=1;desc2.Flags=D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET|D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
 ID3D12Resource *res=0,*opened=0;HANDLE shared=0;
 CHECK(ID3D12Device_CreateCommittedResource(dev,&heap,D3D12_HEAP_FLAG_SHARED,&desc2,D3D12_RESOURCE_STATE_COPY_DEST,0,&IID_ID3D12Resource,(void**)&res));
 CHECK(ID3D12Device_CreateSharedHandle(dev,(ID3D12DeviceChild*)res,0,GENERIC_ALL,0,&shared));
 CHECK(ID3D12Device_OpenSharedHandle(dev,shared,&IID_ID3D12Resource,(void**)&opened));
 printf("Shared texture creation/export/import succeeded\n");CloseHandle(shared);ID3D12Resource_Release(opened);ID3D12Resource_Release(res);return 0;
}
void mainCRTStartup(void) { ExitProcess(probe()); }
