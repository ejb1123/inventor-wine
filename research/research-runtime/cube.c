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
 args[1].vt=VT_BSTR; args[1].bstrVal=SysAllocString(L"C:\\users\\Public\\Documents\\Autodesk\\Inventor 2027\\Templates\\en-US\\Standard.ipt");
 args[2].vt=VT_I4; args[2].lVal=12290;
 VARIANT doc=invoke(docs.pdispVal,L"Add",DISPATCH_METHOD,3,args); out("Created part\r\n");

 VARIANT cd=invoke(doc.pdispVal,L"ComponentDefinition",DISPATCH_PROPERTYGET,0,0);
 VARIANT planes=invoke(cd.pdispVal,L"WorkPlanes",DISPATCH_PROPERTYGET,0,0);
 VARIANT a[4]; for(int i=0;i<4;i++) VariantInit(&a[i]); a[0].vt=VT_I4; a[0].lVal=3;
 VARIANT plane=invoke(planes.pdispVal,L"Item",DISPATCH_PROPERTYGET,1,a);
 VARIANT sketches=invoke(cd.pdispVal,L"Sketches",DISPATCH_PROPERTYGET,0,0);
 a[0].vt=VT_BOOL; a[0].boolVal=VARIANT_FALSE; a[1]=plane;
 VARIANT sketch=invoke(sketches.pdispVal,L"Add",DISPATCH_METHOD,2,a);
 VARIANT tg=invoke(app,L"TransientGeometry",DISPATCH_PROPERTYGET,0,0);
 a[0].vt=VT_R8; a[0].dblVal=0; a[1]=a[0];
 VARIANT p1=invoke(tg.pdispVal,L"CreatePoint2d",DISPATCH_METHOD,2,a);
 a[0].dblVal=2; a[1]=a[0];
 VARIANT p2=invoke(tg.pdispVal,L"CreatePoint2d",DISPATCH_METHOD,2,a);
 VARIANT lines=invoke(sketch.pdispVal,L"SketchLines",DISPATCH_PROPERTYGET,0,0);
 a[0]=p2; a[1]=p1; invoke(lines.pdispVal,L"AddAsTwoPointRectangle",DISPATCH_METHOD,2,a);
 VARIANT profiles=invoke(sketch.pdispVal,L"Profiles",DISPATCH_PROPERTYGET,0,0);
 VARIANT profile=invoke(profiles.pdispVal,L"AddForSolid",DISPATCH_METHOD,0,0);
 VARIANT features=invoke(cd.pdispVal,L"Features",DISPATCH_PROPERTYGET,0,0);
 VARIANT extrudes=invoke(features.pdispVal,L"ExtrudeFeatures",DISPATCH_PROPERTYGET,0,0);
 a[0].vt=VT_I4; a[0].lVal=20481; a[1].vt=VT_I4; a[1].lVal=20993; a[2].vt=VT_R8; a[2].dblVal=2; a[3]=profile;
 invoke(extrudes.pdispVal,L"AddByDistanceExtent",DISPATCH_METHOD,4,a);
 out("Created 20 mm cube\r\n");
 VARIANT view=invoke(app,L"ActiveView",DISPATCH_PROPERTYGET,0,0);
 invoke(view.pdispVal,L"Fit",DISPATCH_METHOD,0,0);
 args[0].boolVal=VARIANT_FALSE; SysFreeString(args[1].bstrVal);
 args[1].bstrVal=SysAllocString(L"C:\\users\\ej\\Documents\\WineResearchCube.ipt");
 invoke(doc.pdispVal,L"SaveAs",DISPATCH_METHOD,2,args); out("Saved part\r\n"); ExitProcess(0);
}
