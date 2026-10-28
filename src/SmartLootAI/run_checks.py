from pathlib import Path
import subprocess,sys,json,hashlib,time
r=Path(__file__).parent
tcc=Path(r'S:\SR\toolchain\tcc\tcc.exe')
cases=[('core','src/test_v2.c'),('fire-guard','src/test_fire_guard.c'),('service-api','src/test_service_api.c'),('native-gates','src/test_native_gates.c'),('fleet-registry','tests/test_fleet_registry.c'),('service-primitives','tests/test_service_policy.c')]
results=[]
for name,src in cases:
 out=r/'tests'/('check-'+name+'.exe')
 subprocess.run([str(tcc),'-Wall','-o',str(out),str(r/src),'-lkernel32'],check=True,capture_output=True)
 p=subprocess.run([str(out)],capture_output=True,timeout=30)
 log=r/'evidence'/('check-'+name+'.log');log.write_bytes(p.stdout+p.stderr)
 assert p.returncode==0 and b'PASS' in p.stdout,(name,p.returncode,log)
 results.append({'name':name,'passed':True,'log_sha256':hashlib.sha256(log.read_bytes()).hexdigest()})
receipt={'tests':results,'dll_sha256':hashlib.sha256((r/'src/SmartLootAI.XenoPlugin.dll').read_bytes()).hexdigest(),'source_sha256':{str(p.relative_to(r)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted((r/'src').glob('*.h'))+sorted((r/'src').glob('*.c'))},'gameplay_tested':False}
(r/'evidence/verification.json').write_text(json.dumps(receipt,indent=2),encoding='utf-8')
print(f'PASS {len(results)} focused x86 test suites; exact DLL/source/log hashes saved.')
