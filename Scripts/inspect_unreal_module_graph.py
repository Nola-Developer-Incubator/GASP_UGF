import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/unreal_module_graph.txt')
with output.open('w', encoding='utf-8') as f:
    for name in sorted(dir(unreal)):
        if 'Graph' in name or 'K2' in name or 'Node' in name or 'Blueprint' in name:
            f.write(name + '\n')
print('wrote', output)
