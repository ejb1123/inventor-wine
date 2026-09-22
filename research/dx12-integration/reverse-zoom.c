#define COBJMACROS
#include <windows.h>
#include <oleauto.h>
static void out(const char *s) { DWORD n; WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,lstrlenA(s),&n,0); }
static void check(HRESULT h,const char *where) { char b[256]; wsprintfA(b,"%s: 0x%08lx\r\n",where,h); out(b); if(FAILED(h)) ExitProcess(1); }
static VARIANT invoke(IDispatch *obj,WCHAR *name,WORD kind,UINT count,VARIANT *args) {
 DISPID id; VARIANT result; VariantInit(&result); check(IDispatch_GetIDsOfNames(obj,&IID_NULL,&name,1,LOCALE_USER_DEFAULT,&id),"get member");
 DISPID put=DISPID_PROPERTYPUT; DISPPARAMS p={args,kind==DISPATCH_PROPERTYPUT ? &put : 0,count,kind==DISPATCH_PROPERTYPUT ? 1 : 0}; EXCEPINFO e={0}; UINT bad=0;
 HRESULT hr=IDispatch_Invoke(obj,id,&IID_NULL,LOCALE_USER_DEFAULT,kind,&p,&result,&e,&bad);
 check(hr,"invoke"); return result;
}
void mainCRTStartup(void) {
 CLSID cls; IUnknown *unknown; IDispatch *app;
 check(CoInitialize(0),"CoInitialize"); check(CLSIDFromProgID(L"Inventor.Application",&cls),"CLSID");
 check(GetActiveObject(&cls,0,&unknown),"GetActiveObject");
 check(IUnknown_QueryInterface(unknown,&IID_IDispatch,(void **)&app),"IDispatch");
 VARIANT opts=invoke(app,L"DisplayOptions",DISPATCH_PROPERTYGET,0,0);
 VARIANT before=invoke(opts.pdispVal,L"ReverseZoomDirection",DISPATCH_PROPERTYGET,0,0);
 if(before.vt!=VT_BOOL) ExitProcess(2);
 char b[256]; wsprintfA(b,"ReverseZoomDirection before=%d\r\n",before.boolVal); out(b);
 VARIANT next; VariantInit(&next); next.vt=VT_BOOL; next.boolVal=before.boolVal ? VARIANT_FALSE : VARIANT_TRUE;
 invoke(opts.pdispVal,L"ReverseZoomDirection",DISPATCH_PROPERTYPUT,1,&next);
 VARIANT after=invoke(opts.pdispVal,L"ReverseZoomDirection",DISPATCH_PROPERTYGET,0,0);
 wsprintfA(b,"ReverseZoomDirection after=%d\r\n",after.boolVal);out(b);
 ExitProcess(after.vt==VT_BOOL && after.boolVal==next.boolVal ? 0 : 3);
}
