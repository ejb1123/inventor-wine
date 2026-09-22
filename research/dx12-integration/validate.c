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

 VARIANT doc=invoke(app,L"ActiveDocument",DISPATCH_PROPERTYGET,0,0);
 VARIANT view=invoke(app,L"ActiveView",DISPATCH_PROPERTYGET,0,0);
 VARIANT cam=invoke(view.pdispVal,L"Camera",DISPATCH_PROPERTYGET,0,0);
 WCHAR *name=L"ViewOrientationType";DISPID id,put=DISPID_PROPERTYPUT;
 check(IDispatch_GetIDsOfNames(cam.pdispVal,&IID_NULL,&name,1,LOCALE_USER_DEFAULT,&id),"orientation property");
 VARIANT v;VariantInit(&v);v.vt=VT_I4;v.lVal=10760;DISPPARAMS dp={&v,&put,1,1};
 check(IDispatch_Invoke(cam.pdispVal,id,&IID_NULL,LOCALE_USER_DEFAULT,DISPATCH_PROPERTYPUT,&dp,0,0,0),"set alternate isometric view");
 invoke(cam.pdispVal,L"Fit",DISPATCH_METHOD,0,0);invoke(cam.pdispVal,L"Apply",DISPATCH_METHOD,0,0);out("Camera updated\r\n");
 VARIANT args[2];VariantInit(&args[0]);VariantInit(&args[1]);args[0].vt=VT_BOOL;args[0].boolVal=VARIANT_TRUE;
 args[1].vt=VT_BSTR;args[1].bstrVal=SysAllocString(L"C:/users/ej/Documents/WineResearchDX12Gear-20260922-0514.ipt");
 invoke(doc.pdispVal,L"SaveAs",DISPATCH_METHOD,2,args);out("Saved test copy\r\n");
 VARIANT docs=invoke(app,L"Documents",DISPATCH_PROPERTYGET,0,0);
 invoke(docs.pdispVal,L"Open",DISPATCH_METHOD,2,args);out("Opened saved test copy\r\n");ExitProcess(0);}
