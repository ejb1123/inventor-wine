#define COBJMACROS
#include <windows.h>
#include <oleauto.h>
static void out(const char *s) { DWORD n; WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,lstrlenA(s),&n,0); }
static void check(HRESULT h,const char *where) { char b[256]; wsprintfA(b,"%s: 0x%08lx\r\n",where,h); out(b); if(FAILED(h)) ExitProcess(1); }
static VARIANT invoke(IDispatch *obj,WCHAR *name,WORD kind,UINT count,VARIANT *args) {
 DISPID id; VARIANT result; VariantInit(&result); check(IDispatch_GetIDsOfNames(obj,&IID_NULL,&name,1,LOCALE_USER_DEFAULT,&id),"get member");
 DISPPARAMS p={args,0,count,0}; EXCEPINFO e={0}; UINT bad=0;
 HRESULT hr=IDispatch_Invoke(obj,id,&IID_NULL,LOCALE_USER_DEFAULT,kind,&p,&result,&e,&bad);
 check(hr,"invoke"); return result;
}
void mainCRTStartup(void) {
 CLSID cls; IUnknown *unknown; IDispatch *app;
 check(CoInitialize(0),"CoInitialize"); check(CLSIDFromProgID(L"Inventor.Application",&cls),"CLSID");
 check(GetActiveObject(&cls,0,&unknown),"GetActiveObject");
 check(IUnknown_QueryInterface(unknown,&IID_IDispatch,(void **)&app),"IDispatch");
 VARIANT hw=invoke(app,L"HardwareOptions",DISPATCH_PROPERTYGET,0,0);
 ITypeInfo *ti; check(IDispatch_GetTypeInfo(hw.pdispVal,0,LOCALE_USER_DEFAULT,&ti),"type info");
 ITypeLib *lib; UINT idx; ITypeInfo_GetContainingTypeLib(ti,&lib,&idx);
 for(UINT i=0;i<ITypeLib_GetTypeInfoCount(lib);i++) { TYPEKIND k; ITypeLib_GetTypeInfoType(lib,i,&k); if(k!=TKIND_ENUM) continue; ITypeInfo *et; ITypeLib_GetTypeInfo(lib,i,&et); BSTR en; ITypeInfo_GetDocumentation(et,MEMBERID_NIL,&en,0,0,0); char b[512]; WideCharToMultiByte(CP_UTF8,0,en,-1,b,512,0,0); if(lstrcmpW(en,L"ViewOrientationTypeEnum") && lstrcmpW(en,L"ViewOrientationTypeEnum")) continue; out(b); out("\r\n"); TYPEATTR *ta; ITypeInfo_GetTypeAttr(et,&ta); for(UINT j=0;j<ta->cVars;j++) { VARDESC *v; ITypeInfo_GetVarDesc(et,j,&v); BSTR name; UINT n; ITypeInfo_GetNames(et,v->memid,&name,1,&n); char nm[256]; WideCharToMultiByte(CP_UTF8,0,name,-1,nm,256,0,0); wsprintfA(b,"%s = %ld\r\n",nm,v->lpvarValue->lVal); out(b); } }
 ExitProcess(0);
}
