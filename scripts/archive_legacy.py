"""Copy only original source/UI files; never mutate the backup."""
from pathlib import Path
import hashlib, json, shutil

root = Path(__file__).resolve().parents[1]
source = Path(r'E:\--Backup--\手动备份\自写程序备份\自写程序备份\QT Project')
projects = {'os': source/'可视化操作系统'/'OS', 'huffman': source/'huffman 2.4 10M-3.2-0.8', 'calculator': source/'calculator'}
records = []
for name, folder in projects.items():
    for file in sorted(folder.iterdir()):
        if file.is_file() and file.suffix.lower() in {'.cpp', '.h', '.ui', '.pro', '.qrc'} and not file.name.startswith('ui_'):
            target = root/'legacy'/name/file.name
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(file, target)
            data = target.read_bytes()
            records.append({'app': name, 'source': str(file), 'archive': str(target.relative_to(root)).replace('\\','/'), 'bytes':len(data), 'sha256':hashlib.sha256(data).hexdigest()})
(root/'legacy'/'manifest.json').write_text(json.dumps({'copied_source_only':True,'backup_read_only':True,'files':records},ensure_ascii=False,indent=2),encoding='utf-8')
print(f'Archived {len(records)} original source files')
