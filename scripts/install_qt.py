"""Install official Qt base archive with upstream SHA-1 verification.

Qt 6.12 repositories use per-architecture folders, which aqtinstall 3.3.0
does not yet resolve. No Qt account, credentials, or old editor is needed.
"""
from pathlib import Path
import hashlib, json, requests, xml.etree.ElementTree as ET
import py7zr

root=Path(__file__).resolve().parents[1]/'.toolchain'
base='https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt6_6120/qt6_6120_msvc2022_64/'
session=requests.Session()
response=session.get(base+'Updates.xml',timeout=40); response.raise_for_status()
xml=ET.fromstring(response.content)
package=next(p for p in xml.findall('PackageUpdate') if p.findtext('Name')=='qt.qt6.6120.win64_msvc2022_64')
name=next(n.strip() for n in package.findtext('DownloadableArchives').split(',') if n.strip().startswith('qtbase-'))
url=base+package.findtext('Name')+'/'+package.findtext('Version')+name
checksum_response=session.get(url+'.sha1',timeout=40); checksum_response.raise_for_status()
checksum=checksum_response.text.strip().split()[0].lower()
archive=root/name
if not archive.exists() or hashlib.sha1(archive.read_bytes()).hexdigest()!=checksum:
    with session.get(url,stream=True,timeout=(20,90)) as response:
        response.raise_for_status(); total=int(response.headers.get('Content-Length',0)); done=0; report=0
        with archive.open('wb') as output:
            for chunk in response.iter_content(1024*1024):
                output.write(chunk); done+=len(chunk)
                if done-report>=16*1024*1024: print(f'Downloaded {done/1024/1024:.1f} / {total/1024/1024:.1f} MiB',flush=True); report=done
actual=hashlib.sha1(archive.read_bytes()).hexdigest()
if actual!=checksum: raise RuntimeError(f'Archive checksum mismatch: {actual} != {checksum}')
destination=root/'Qt'/'6.12.0'/'msvc2022_64'
destination.mkdir(parents=True,exist_ok=True)
print(f'Verified {checksum}; extracting to {destination}',flush=True)
with py7zr.SevenZipFile(archive,'r') as z:
    names=z.getnames()
    if any(Path(n).is_absolute() or '..' in Path(n).parts for n in names): raise RuntimeError('Unsafe archive path')
    z.extractall(destination)
(root/'qt-installation.json').write_text(json.dumps({'version':'6.12.0','architecture':'msvc2022_64','url':url,'sha1':actual,'sha256':hashlib.sha256(archive.read_bytes()).hexdigest(),'path':str(destination)},indent=2),encoding='utf-8')
print('Qt base ready',flush=True)
