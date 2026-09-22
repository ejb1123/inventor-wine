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
 VARIANT args[3]; for(int i=0;i<3;i++) VariantInit(&args[i]);
 args[0].vt=VT_BOOL; args[0].boolVal=VARIANT_TRUE;
 args[1].vt=VT_BSTR; args[1].bstrVal=SysAllocString(L"C:\\users\\Public\\Documents\\Autodesk\\Inventor 2027\\Web\\bevel_gear-1.ipt");
 args[2].vt=VT_I4; args[2].lVal=12290;
 VARIANT doc=invoke(docs.pdispVal,L"Open",DISPATCH_METHOD,2,args); out("Opened sample gear\r\n");
 args[0].boolVal=VARIANT_FALSE; SysFreeString(args[1].bstrVal);
 args[1].bstrVal=SysAllocString(L"C:\\users\\ej\\Documents\\WineResearchGear.ipt");
 invoke(doc.pdispVal,L"SaveAs",DISPATCH_METHOD,2,args); out("Saved part\r\n"); ExitProcess(0);
}
