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
 VARIANT cm=invoke(app,L"CommandManager",DISPATCH_PROPERTYGET,0,0);VARIANT defs=invoke(cm.pdispVal,L"ControlDefinitions",DISPATCH_PROPERTYGET,0,0);
 WCHAR *names[]={L"AppRealisticShadingDisplayCmd",L"AppUseRapidRTToggleCmd"};for(int i=0;i<2;i++){VARIANT n;VariantInit(&n);n.vt=VT_BSTR;n.bstrVal=SysAllocString(names[i]);VARIANT d=invoke(defs.pdispVal,L"Item",DISPATCH_PROPERTYGET,1,&n);invoke(d.pdispVal,L"Execute",DISPATCH_METHOD,0,0);}
 ExitProcess(0);}
