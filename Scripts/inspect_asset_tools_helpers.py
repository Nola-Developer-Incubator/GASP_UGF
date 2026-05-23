import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/asset_tools_helpers.txt')
with output.open('w', encoding='utf-8') as f:
    ath = unreal.AssetToolsHelpers
    for name in sorted(dir(ath)):
        if not name.startswith('_'):
            f.write(name + '\n')
    f.write('\n-- get_asset_tools type --\n')
    try:
        tools = ath.get_asset_tools()
        f.write('type: %s\n' % type(tools).__name__)
        for name in sorted(dir(tools)):
            if not name.startswith('_'):
                f.write(name + '\n')
    except Exception as e:
        f.write('error: %s\n' % e)
print('done')
