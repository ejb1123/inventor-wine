/* Export saved Inventor documents to STEP for independent geometry checks.
 * Uses the STEP TranslatorAddIn in a running disposable Inventor session.
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


static VARIANT disp(IDispatch*d){VARIANT v;VariantInit(&v);v.vt=VT_DISPATCH;v.pdispVal=d;return v;}
static IDispatch* typed(IDispatch*o,WCHAR*name){ITypeLib*lib=0;HRESULT hr=LoadTypeLibEx(L"C:\\Program Files\\Autodesk\\Inventor 2027\\Bin\\RxInventor.tlb",REGKIND_NONE,&lib);if(FAILED(hr))fail(hr,L"tlb");for(UINT i=0;i<ITypeLib_GetTypeInfoCount(lib);i++){BSTR n=0;ITypeLib_GetDocumentation(lib,i,&n,0,0,0);if(n&&!lstrcmpW(n,name)){ITypeInfo*ti=0;TYPEATTR*attr=0;ITypeLib_GetTypeInfo(lib,i,&ti);ITypeInfo_GetTypeAttr(ti,&attr);IDispatch*r=0;hr=IDispatch_QueryInterface(o,&attr->guid,(void**)&r);if(FAILED(hr))fail(hr,L"typed QI");return r;}SysFreeString(n);}fail(E_FAIL,L"type missing");return 0;}
static void export_step(IDispatch*app,IDispatch*d,WCHAR*path){
 IDispatch*adds=get(app,L"ApplicationAddIns");VARIANT id=str(L"{90AF7F40-0C01-11D5-8E83-0010B541CD80}");IDispatch*t=obj(call(adds,L"ItemById",DISPATCH_PROPERTYGET,1,&id));
 t=typed(t,L"TranslatorAddIn");
 IDispatch*to=get(app,L"TransientObjects");IDispatch*ctx=obj(call(to,L"CreateTranslationContext",DISPATCH_METHOD,0,0));IDispatch*opts=obj(call(to,L"CreateNameValueMap",DISPATCH_METHOD,0,0));
 VARIANT typ=integer(13059);call(ctx,L"Type",DISPATCH_PROPERTYPUT,1,&typ);
 VARIANT h[3]={disp(opts),disp(ctx),disp(d)};call(t,L"HasSaveCopyAsOptions",DISPATCH_PROPERTYGET,3,h);
 IDispatch*data=obj(call(to,L"CreateDataMedium",DISPATCH_METHOD,0,0));VARIANT file=str(path);call(data,L"FileName",DISPATCH_PROPERTYPUT,1,&file);
 VARIANT a[4]={disp(data),disp(opts),disp(ctx),disp(d)};call(t,L"SaveCopyAs",DISPATCH_METHOD,4,a);
}
void mainCRTStartup(void){CoInitialize(0);CLSID cls;IUnknown*u=0;IDispatch*app=0;CLSIDFromProgID(L"Inventor.Application",&cls);HRESULT hr=GetActiveObject(&cls,0,&u);if(FAILED(hr))fail(hr,L"active");IUnknown_QueryInterface(u,&IID_IDispatch,(void**)&app);IDispatch*docs=get(app,L"Documents");
 WIN32_FIND_DATAW fd;HANDLE h=FindFirstFileW(L"C:\\ResearchUI\\flock-native\\native\\*.ipt",&fd);if(h==INVALID_HANDLE_VALUE)fail(E_FAIL,L"find");
 do{WCHAR src[MAX_PATH],dst[MAX_PATH];wsprintfW(src,L"C:\\ResearchUI\\flock-native\\native\\%s",fd.cFileName);WCHAR stem[MAX_PATH];lstrcpyW(stem,fd.cFileName);stem[lstrlenW(stem)-4]=0;wsprintfW(dst,L"C:\\ResearchUI\\flock-native\\roundtrip\\%s.step",stem);IDispatch*d=open_doc(docs,src);printprop(d,L"FullFileName");export_step(app,d,dst);close_doc(d);}while(FindNextFileW(h,&fd));FindClose(h);
 for(int p=0;p<2;p++){WCHAR*src=p?L"C:\\ResearchUI\\coverage\\flock-portable\\solar-site\\falcon-solar-site-assembly.iam":L"C:\\ResearchUI\\coverage\\flock-portable\\camera\\falcon-camera-assembly.iam";WCHAR*dst=p?L"C:\\ResearchUI\\flock-native\\roundtrip\\solar.step":L"C:\\ResearchUI\\flock-native\\roundtrip\\camera.step";IDispatch*d=open_doc(docs,src);export_step(app,d,dst);close_doc(d);}
 out("EXPORT COMPLETE\r\n");ExitProcess(0);}
