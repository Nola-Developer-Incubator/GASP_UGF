import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/export_api.txt')
with output.open('w', encoding='utf-8') as f:
    for name in sorted(dir(unreal)):
        if 'Export' in name or 'AssetExport' in name or 'ExportTask' in name or 'Exporter' in name:
            f.write(name + '\n')
print('wrote', output)
