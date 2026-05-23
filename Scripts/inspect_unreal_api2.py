import unreal

print('Loaded unreal version:', unreal.SystemLibrary.get_engine_version())
print('BlueprintEditorLibrary members:')
for name in sorted(dir(unreal.BlueprintEditorLibrary)):
    if not name.startswith('_'):
        print(name)

print('\nEditorAssetLibrary members:')
for name in sorted(dir(unreal.EditorAssetLibrary)):
    if not name.startswith('_'):
        print(name)
