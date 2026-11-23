/* Bounded interactive coverage driver. Mutations require a disposable test
 * copy. Build with MinGW, -nostdlib, entry mainCRTStartup,
 * ole32/oleaut32/uuid/user32/shell32/kernel32. API references and usage are in
 * coverage.md. Does not save documents. */
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
void mainCRTStartup(void) {
  int argc;
  WCHAR **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (argc < 2)
    fail(E_INVALIDARG, L"command");
  HRESULT hr = CoInitialize(0);
  if (FAILED(hr))
    fail(hr, L"CoInitialize");
  CLSID cls;
  IUnknown *u = 0;
  IDispatch *app = 0;
  hr = CLSIDFromProgID(L"Inventor.Application", &cls);
  if (FAILED(hr))
    fail(hr, L"CLSID");
  hr = GetActiveObject(&cls, 0, &u);
  if (FAILED(hr))
    fail(hr, L"GetActiveObject");
  if (!u)
    fail(E_POINTER, L"GetActiveObject returned null");
  hr = IUnknown_QueryInterface(u, &IID_IDispatch, (void **)&app);
  if (FAILED(hr))
    fail(hr, L"IDispatch");
  IUnknown_Release(u);
  if (!lstrcmpW(argv[1], L"open") && argc == 3) {
    WCHAR prefix[] = L"C:\\ResearchUI\\coverage\\";
    if (CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, argv[2],
                       lstrlenW(prefix), prefix, -1) != CSTR_EQUAL)
      fail(E_ACCESSDENIED, L"test copy path required");
    VARIANT a[2];
    VariantInit(&a[0]);
    VariantInit(&a[1]);
    a[0].vt = VT_BOOL;
    a[0].boolVal = VARIANT_TRUE;
    a[1].vt = VT_BSTR;
    a[1].bstrVal = SysAllocString(argv[2]);
    IDispatch *docs = get(app, L"Documents");
    IDispatch *doc = obj(call(docs, L"Open", DISPATCH_METHOD, 2, a));
    call(doc, L"Activate", DISPATCH_METHOD, 0, 0);
    printprop(doc, L"FullFileName");
  } else if (!lstrcmpW(argv[1], L"assets")) {
    IDispatch *libs = get(app, L"AssetLibraries");
    LONG count = call(libs, L"Count", DISPATCH_PROPERTYGET, 0, 0).lVal;
    for (LONG i = 1; i <= count; i++) {
      IDispatch *lib = item(libs, i);
      printprop(lib, L"DisplayName");
      IDispatch *assets = get(lib, L"AppearanceAssets");
      LONG ac = call(assets, L"Count", DISPATCH_PROPERTYGET, 0, 0).lVal;
      for (LONG j = 1; j <= ac; j++) {
        char b[100];
        wsprintfA(b, "library=%ld asset=%ld ", i, j);
        out(b);
        IDispatch *a = item(assets, j);
        printprop(a, L"DisplayName");
        IDispatch_Release(a);
      }
      IDispatch_Release(assets);
      IDispatch_Release(lib);
    }
  } else {
    require_copy(app);
    IDispatch *view = get(app, L"ActiveView");
    if (!lstrcmpW(argv[1], L"camera") && argc == 3) {
      VARIANT v = integer(number(argv[2]));
      if (v.lVal < 10754 || v.lVal > 10764)
        fail(E_INVALIDARG, L"orientation");
      IDispatch *cam = get(view, L"Camera");
      call(cam, L"ViewOrientationType", DISPATCH_PROPERTYPUT, 1, &v);
      call(cam, L"Fit", DISPATCH_METHOD, 0, 0);
      call(cam, L"ApplyWithoutTransition", DISPATCH_METHOD, 0, 0);
      IDispatch_Release(cam);
    } else if (!lstrcmpW(argv[1], L"appearance") && argc == 4) {
      IDispatch *libs = get(app, L"AssetLibraries"),
                *lib = item(libs, number(argv[2])),
                *assets = get(lib, L"AppearanceAssets"),
                *asset = item(assets, number(argv[3])),
                *doc = get(app, L"ActiveDocument");
      VARIANT copy;
      VariantInit(&copy);
      copy.vt = VT_DISPATCH;
      copy.pdispVal = asset;
      call(doc, L"ActiveAppearance", DISPATCH_PROPERTYPUT, 1, &copy);
      printprop(asset, L"DisplayName");
    } else if (!lstrcmpW(argv[1], L"observe")) {
      for (int i = 0; i < 60; i++) {
        char b[50];
        wsprintfA(b, "sample=%d\r\n", i);
        out(b);
        printprop(view, L"RayTracingProgress");
        printprop(view, L"IsRayTracingPaused");
        Sleep(1000);
      }
    } else
      fail(E_INVALIDARG, L"command");
    printprop(view, L"RayTracing");
    IDispatch_Release(view);
  }
  out("completed\r\n");
  IDispatch_Release(app);
  CoUninitialize();
  ExitProcess(0);
}
