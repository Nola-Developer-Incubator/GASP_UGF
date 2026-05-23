import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/unreal_api_list.txt')
with output.open('w', encoding='utf-8') as f:
    f.write('Loaded unreal version: %s\n' % unreal.SystemLibrary.get_engine_version())
    f.write('BlueprintEditorLibrary members:\n')
    for name in sorted(dir(unreal.BlueprintEditorLibrary)):
        if not name.startswith('_'):
            f.write(name + '\n')
    f.write('\nEditorAssetLibrary members:\n')
    for name in sorted(dir(unreal.EditorAssetLibrary)):
        if not name.startswith('_'):
            f.write(name + '\n')
print('wrote', output)
