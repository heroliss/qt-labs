"""Obtain exact unmodified Qt source and its bundled licence notices."""
from pathlib import Path
import hashlib,json,tarfile,requests

repo=Path(__file__).resolve().parents[1]
work=repo/'.toolchain'
url='https://download.qt.io/official_releases/qt/6.12/6.12.0/submodules/qtbase-everywhere-src-6.12.0.tar.xz'
session=requests.Session()
response=session.get(url+'.sha256',timeout=40);response.raise_for_status();expected=response.text.strip().split()[0].lower()
archive=work/'qtbase-everywhere-src-6.12.0.tar.xz'
if not archive.exists() or hashlib.sha256(archive.read_bytes()).hexdigest()!=expected:
    with session.get(url,stream=True,timeout=(20,90)) as response:
        response.raise_for_status()
        with archive.open('wb') as output:
            for chunk in response.iter_content(1024*1024):output.write(chunk)
actual=hashlib.sha256(archive.read_bytes()).hexdigest()
if actual!=expected:raise RuntimeError('Qt source checksum mismatch')
target=repo/'third-party'/'qt-6.12.0'
target.mkdir(parents=True,exist_ok=True)
records=[]
with tarfile.open(archive,'r:xz') as tar:
    for member in tar.getmembers():
        path=Path(member.name)
        # Keep all upstream top-level licences, bundled third-party notices and attribution metadata.
        lower=path.name.lower()
        allowed=('LICENSES' in path.parts or ('3rdparty' in path.parts and (lower.startswith(('license','copying','copyright','notice','readme')) or lower=='qt_attribution.json')))
        if member.isfile() and allowed and member.size<=2*1024*1024:
            relative=Path(*path.parts[1:])
            if '..' in relative.parts or relative.is_absolute():raise RuntimeError('Unsafe source path')
            output=target/relative;output.parent.mkdir(parents=True,exist_ok=True);data=tar.extractfile(member).read();output.write_bytes(data)
            records.append({'path':str(output.relative_to(repo)).replace('\\','/'),'sha256':hashlib.sha256(data).hexdigest()})
(target/'source.json').write_text(json.dumps({'version':'6.12.0','source_url':url,'source_sha256':actual,'unmodified_qt':True,'dynamic_linking':True,'notices':records},indent=2),encoding='utf-8')
print(json.dumps({'archive':str(archive),'sha256':actual,'notices':len(records)},indent=2))
