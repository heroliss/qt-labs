"""Check raw original bytes in the working tree and committed Git blobs."""
from pathlib import Path
import hashlib,json,subprocess,sys

root=Path(__file__).resolve().parents[1]
manifest=json.loads((root/'legacy/manifest.json').read_text(encoding='utf-8'))
ref=sys.argv[1] if len(sys.argv)>1 else None
results=[]
for record in manifest['files']:
    data=(root/record['archive']).read_bytes()
    actual=hashlib.sha256(data).hexdigest()
    result={'path':record['archive'],'workspace':actual==record['sha256'],'sha256':actual}
    if ref:
        blob=subprocess.check_output(['git','cat-file','blob',f"{ref}:{record['archive']}"],cwd=root)
        result['git_blob']=hashlib.sha256(blob).hexdigest()==record['sha256']
    results.append(result)
licenses=json.loads((root/'third-party/qt-6.12.0/source.json').read_text(encoding='utf-8'))
license_results=[]
for record in licenses['notices']:
    data=(root/record['path']).read_bytes()
    result={'path':record['path'],'workspace':hashlib.sha256(data).hexdigest()==record['sha256']}
    if ref:
        blob=subprocess.check_output(['git','cat-file','blob',f"{ref}:{record['path']}"],cwd=root)
        result['git_blob']=hashlib.sha256(blob).hexdigest()==record['sha256']
    license_results.append(result)
okay=all(x['workspace'] and x.get('git_blob',True) for x in results+license_results)
print(json.dumps({'passed':okay,'ref':ref,'original_files':results,'upstream_notices':license_results},ensure_ascii=False,indent=2))
raise SystemExit(0 if okay else 1)
