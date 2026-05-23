import unreal, pathlib, inspect
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
with open(r'C:/Unreal_Projects/GASP/Scripts/export_assets_sig.txt', 'w', encoding='utf-8') as f:
    fn = getattr(asset_tools, 'export_assets')
    f.write('callable: %s\n' % fn)
    try:
        f.write('doc:\n%s\n' % (fn.__doc__ or 'NO DOC'))
    except Exception as e:
        f.write('doc error: %s\n' % e)
    try:
        sig = inspect.signature(fn)
        f.write('signature: %s\n' % sig)
    except Exception as e:
        f.write('signature error: %s\n' % e)
