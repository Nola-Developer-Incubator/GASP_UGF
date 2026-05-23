import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/exporter2.txt')
exp = unreal.Exporter()
with output.open('w', encoding='utf-8') as f:
    for name in sorted(dir(exp)):
        if not name.startswith('_'):
            f.write(name + '\n')
print('wrote', output)
