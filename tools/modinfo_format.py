"""Readable ModuleInfo descriptions for Vityargh mods.

The game ignores a doubled <br><br> (no blank line), so sections ran together.
Format applied to FullDescription / FullDescriptionEng:
  <color=0,80,170>Heading</color>
  - body text
  (spacer line)
"""
import re, sys, zipfile, shutil
from pathlib import Path

COLOR = '0,80,170'
SPACER = '<br><br>'  # standalone line, as in Kotyanka mods


def format_text(text: str) -> str:
    out = []
    lines = text.replace('\r\n', '\n').split('\n')
    for line in lines:
        m = re.match(r'^(FullDescription(?:Eng)?)=(.*)$', line)
        if not m:
            out.append(line); continue
        key, val = m.groups()
        if val.strip() in ('', '<br>', '<br><br>', '\xa0<br>', '&nbsp;<br>'):
            continue  # old spacers are rebuilt below
        raw = re.sub(r'(<br>)+\s*$', '', val).strip()
        plain = re.sub(r'</?color[^>]*>', '', raw).strip()
        if re.sub(r'<[^>]+>', '', plain).startswith('- '):
            plain = plain[2:]
        is_heading = val.rstrip().endswith('<br>') and not val.rstrip().endswith('<br><br>') and len(plain) <= 40
        if is_heading:
            if out and re.match(rf'^{key}=- ', out[-1]):
                out.append(f'{key}={SPACER}')
            out.append(f'{key}=<color={COLOR}>{plain}</color><br>')
        else:
            out.append(f'{key}=- {plain}<br>')
    return '\r\n'.join(l for l in out).rstrip('\r\n') + '\r\n'


def format_file(path: Path):
    t = path.read_bytes().decode('cp1251')
    new = format_text(t)
    path.write_bytes(new.encode('cp1251'))
    return new


def patch_zip(zip_path: Path, new_bytes: bytes):
    tmp = zip_path.with_suffix('.tmp.zip')
    with zipfile.ZipFile(zip_path) as old, zipfile.ZipFile(tmp, 'w', zipfile.ZIP_DEFLATED) as z:
        for i in old.infolist():
            data = new_bytes if i.filename.replace('\\', '/').endswith('ModuleInfo.txt') else old.read(i)
            z.writestr(i, data)
    shutil.move(tmp, zip_path)


if __name__ == '__main__':
    for p in sys.argv[1:]:
        print(format_file(Path(p)))
