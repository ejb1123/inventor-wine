from pathlib import Path
import pefile,re,shutil
p=Path.home()/'.local/share/wineprefixes/inventor-2027-research/drive_c'
theme=p/'windows/resources/themes/aero/aero.msstyles'
out=p/'windows/resources/themes/research/research.msstyles';out.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(theme,out)
pe=pefile.PE(str(theme))
for t in pe.DIRECTORY_ENTRY_RESOURCE.entries:
 if str(t.name)!='TEXTFILE':continue
 for n in t.directory.entries:
  if str(n.name)!='BLUE_INI':continue
  d=n.directory.entries[0].data.struct;s=pe.get_data(d.OffsetToData,d.Size).decode('utf-16le')
  def font(m):
   key,size=m.group(1),int(m.group(2))
   if key=='GlyphFont':return m.group(0)
   return f'{key} = Noto Sans, {max(size,10) if size>0 else min(size,-13)}'
  s=re.sub(r'(\w*Font) = Tahoma, (-?\d+)',font,s)
  (p/'ResearchUI/theme.ini').write_bytes(s.encode('utf-16le'))
