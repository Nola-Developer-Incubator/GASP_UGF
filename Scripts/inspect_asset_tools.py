import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/asset_tools.txt')
with output.open('w', encoding='utf-8') as f:
    for name in sorted(dir(unreal)):
        if 'AssetTools' in name or 'AssetRegistry' in name or 'AssetImport' in name or 'Asset' in name and 'Tools' in name:
            f.write(name + '\n')
print('wrote', output)
