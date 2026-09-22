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
 VARIANT docs=invoke(app,L"Documents",DISPATCH_PROPERTYGET,0,0);
 VARIANT count=invoke(docs.pdispVal,L"Count",DISPATCH_PROPERTYGET,0,0); int dirty=0;
 for(LONG i=count.lVal;i>=1;i--){VARIANT index;VariantInit(&index);index.vt=VT_I4;index.lVal=i;VARIANT doc=invoke(docs.pdispVal,L"Item",DISPATCH_PROPERTYGET,1,&index);VARIANT name=invoke(doc.pdispVal,L"DisplayName",DISPATCH_PROPERTYGET,0,0);if(lstrcmpW(name.bstrVal,L"bevel_gear-1.ipt")&&lstrcmpW(name.bstrVal,L"bevel_gear-1")){out("Unexpected document; abort\r\n");ExitProcess(3);}
 VARIANT typ=invoke(doc.pdispVal,L"DocumentType",DISPATCH_PROPERTYGET,0,0);WCHAR path[256];wsprintfW(path,L"C:/users/ej/Documents/RayTest-Recovery-20260922-0555-%ld.%s",i,typ.lVal==12292?L"idw":typ.lVal==12291?L"iam":L"ipt");VARIANT args[2];VariantInit(&args[0]);VariantInit(&args[1]);args[0].vt=VT_BOOL;args[0].boolVal=VARIANT_TRUE;args[1].vt=VT_BSTR;args[1].bstrVal=SysAllocString(path);invoke(doc.pdispVal,L"SaveAs",DISPATCH_METHOD,2,args);invoke(doc.pdispVal,L"Close",DISPATCH_METHOD,1,args);}
 invoke(app,L"Quit",DISPATCH_METHOD,0,0);ExitProcess(0);}
