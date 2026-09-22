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
 TYPEATTR *ta; ITypeInfo_GetTypeAttr(ti,&ta);
 for(UINT i=0;i<ta->cFuncs;i++) { FUNCDESC *f; ITypeInfo_GetFuncDesc(ti,i,&f); BSTR name; UINT got; ITypeInfo_GetNames(ti,f->memid,&name,1,&got); char b[256]; WideCharToMultiByte(CP_UTF8,0,name,-1,b,256,0,0); out(b); out("\r\n"); ITypeInfo_ReleaseFuncDesc(ti,f); }
 ExitProcess(0);
}
