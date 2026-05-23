import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/exporter.txt')
exp = unreal.Exporter()
with output.open('w', encoding='utf-8') as f:
    for name in sorted(dir(exp)):
        if not name.startswith('_'):
            try:
                f.write(name + '\n')
            except Exception:
                pass
print('wrote', output)
