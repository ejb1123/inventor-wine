/* Inventor native document and assembly reference reopen probe.
 * Reference resolution does NOT validate component geometry.
 * Build with MinGW, -nostdlib, entry mainCRTStartup;
 * link ole32/oleaut32/uuid/user32/shell32/kernel32/lgcc. */
#define COBJMACROS
#include <windows.h>

#include <oleauto.h>
#include <shellapi.h>
static void out(const char *s) {
  DWORD n;
  WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), s, lstrlenA(s), &n, 0);
}
static void fail(HRESULT hr, const WCHAR *name) {
  char b[1024];
  wsprintfA(b, "%S failed: %08lx\r\n", name, hr);
  out(b);
  ExitProcess(1);
}
static VARIANT call(IDispatch *o, WCHAR *name, WORD flags, UINT n, VARIANT *a) {
  if (!o)
    fail(E_POINTER, name);
  DISPID id, put = DISPID_PROPERTYPUT;
  HRESULT hr =
      IDispatch_GetIDsOfNames(o, &IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &id);
  if (FAILED(hr))
    fail(hr, name);
  DISPPARAMS dp = {a, 0, n, 0};
  if (flags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF)) {
    dp.rgdispidNamedArgs = &put;
    dp.cNamedArgs = 1;
  }
  EXCEPINFO ex = {0};
  VARIANT v;
  VariantInit(&v);
  hr = IDispatch_Invoke(o, id, &IID_NULL, LOCALE_USER_DEFAULT, flags, &dp, &v,
                        &ex, 0);
  if (FAILED(hr)) {
    if (ex.pfnDeferredFillIn)
      ex.pfnDeferredFillIn(&ex);
    if (ex.bstrDescription) {
      char b[2048];
      wsprintfA(b, "Exception: %.900S\r\n", ex.bstrDescription);
      out(b);
    }
    fail(hr, name);
  }
  return v;
}
static IDispatch *obj(VARIANT v) {
  if (v.vt != VT_DISPATCH || !v.pdispVal)
    fail(E_NOINTERFACE, L"object");
  return v.pdispVal;
}
static VARIANT integer(LONG n) {
  VARIANT v;
  VariantInit(&v);
  v.vt = VT_I4;
  v.lVal = n;
  return v;
}
static LONG number(WCHAR *s) {
  LONG n = 0;
  if (!*s)
    fail(E_INVALIDARG, L"number");
  for (; *s; s++) {
    if (*s < L'0' || *s > L'9' || n > 100000)
      fail(E_INVALIDARG, L"number");
    n = n * 10 + *s - L'0';
  }
  return n;
}
static IDispatch *get(IDispatch *o, WCHAR *n) {
  return obj(call(o, n, DISPATCH_PROPERTYGET, 0, 0));
}
static IDispatch *item(IDispatch *o, LONG n) {
  VARIANT v = integer(n);
  return obj(call(o, L"Item", DISPATCH_PROPERTYGET, 1, &v));
}
static void printprop(IDispatch *o, WCHAR *name) {
  VARIANT v = call(o, name, DISPATCH_PROPERTYGET, 0, 0), s;
  VariantInit(&s);
  HRESULT hr = VariantChangeType(&s, &v, 0, VT_BSTR);
  if (FAILED(hr))
    fail(hr, name);
  char b[2048];
  wsprintfA(b, "%S=%S\r\n", name, s.bstrVal);
  out(b);
  VariantClear(&s);
  VariantClear(&v);
}
static void require_copy(IDispatch *app) {
  IDispatch *doc = get(app, L"ActiveDocument");
  VARIANT p = call(doc, L"FullFileName", DISPATCH_PROPERTYGET, 0, 0);
  WCHAR prefix[] = L"C:\\ResearchUI\\coverage\\";
  if (p.vt != VT_BSTR ||
      CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, p.bstrVal,
                     lstrlenW(prefix), prefix, -1) != CSTR_EQUAL)
    fail(E_ACCESSDENIED, L"disposable document required");
  VariantClear(&p);
  IDispatch_Release(doc);
}

static VARIANT str(WCHAR *s){VARIANT v;VariantInit(&v);v.vt=VT_BSTR;v.bstrVal=SysAllocString(s);return v;}
static VARIANT boolvar(BOOL b){VARIANT v;VariantInit(&v);v.vt=VT_BOOL;v.boolVal=b?VARIANT_TRUE:VARIANT_FALSE;return v;}
static IDispatch *open_doc(IDispatch *docs,WCHAR *path){VARIANT a[2]={boolvar(FALSE),str(path)};return obj(call(docs,L"Open",DISPATCH_METHOD,2,a));}
static void save_doc(IDispatch *doc,WCHAR *path){VARIANT a[2]={boolvar(FALSE),str(path)};call(doc,L"SaveAs",DISPATCH_METHOD,2,a);}
static void close_doc(IDispatch *doc){VARIANT a=boolvar(TRUE);call(doc,L"Close",DISPATCH_METHOD,1,&a);IDispatch_Release(doc);}
static void verify_part(IDispatch *doc){VARIANT t=call(doc,L"DocumentType",DISPATCH_PROPERTYGET,0,0);if(t.lVal!=12290)fail(E_FAIL,L"not a part document");printprop(doc,L"DocumentType");}

void mainCRTStartup(void){CoInitialize(0);CLSID cls;IUnknown*u=0;IDispatch*app=0;
 HRESULT hr=CLSIDFromProgID(L"Inventor.Application",&cls);if(FAILED(hr))fail(hr,L"CLSID");hr=GetActiveObject(&cls,0,&u);if(FAILED(hr))fail(hr,L"active");hr=IUnknown_QueryInterface(u,&IID_IDispatch,(void**)&app);if(FAILED(hr))fail(hr,L"app");
 IDispatch*docs=get(app,L"Documents");
 for(int pass=0;pass<2;pass++){
 WCHAR*path=pass?L"C:\\ResearchUI\\coverage\\flock-portable\\solar-site\\falcon-solar-site-assembly.iam":L"C:\\ResearchUI\\coverage\\flock-portable\\camera\\falcon-camera-assembly.iam";
 IDispatch*d=open_doc(docs,path);printprop(d,L"FullFileName");
 IDispatch*refs=get(d,L"AllReferencedDocuments");LONG n=call(refs,L"Count",DISPATCH_PROPERTYGET,0,0).lVal;printprop(refs,L"Count");
 if(n!=(pass?26:15))fail(E_FAIL,L"reference count");
 for(LONG i=1;i<=n;i++){IDispatch*r=item(refs,i);printprop(r,L"FullFileName");IDispatch_Release(r);}
 close_doc(d);
 }
 out("REFERENCE CHECK PASSED\r\n");ExitProcess(0);}
