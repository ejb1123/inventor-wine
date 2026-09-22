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
 for(LONG i=1;i<=count.lVal;i++) { VARIANT index;VariantInit(&index);index.vt=VT_I4;index.lVal=i;
 VARIANT doc=invoke(docs.pdispVal,L"Item",DISPATCH_PROPERTYGET,1,&index);
 VARIANT name=invoke(doc.pdispVal,L"DisplayName",DISPATCH_PROPERTYGET,0,0);
 char b[1024];WideCharToMultiByte(CP_UTF8,0,name.bstrVal,-1,b,1024,0,0);out(b);
 VARIANT changed=invoke(doc.pdispVal,L"Dirty",DISPATCH_PROPERTYGET,0,0);out(changed.boolVal?" DIRTY\r\n":" SAVED\r\n");if(changed.boolVal)dirty++;
 }
 if(dirty) { out("UNSAVED DOCUMENTS: restart deferred\r\n");ExitProcess(2); }
 invoke(app,L"Quit",DISPATCH_METHOD,0,0);ExitProcess(0);
}
