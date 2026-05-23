import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/asset_export_task.txt')
with output.open('w', encoding='utf-8') as f:
    aet = unreal.AssetExportTask()
    f.write('AssetExportTask type: %s\n' % type(aet).__name__)
    for name in sorted(dir(aet)):
        if not name.startswith('_'):
            try:
                f.write(name + '\n')
            except Exception:
                pass
print('wrote', output)
