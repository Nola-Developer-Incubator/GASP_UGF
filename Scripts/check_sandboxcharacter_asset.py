import unreal
import os

asset_path = '/Game/Blueprints/SandboxCharacter_GASP'
result_path = r'C:\Unreal_Projects\GASP\Scripts\check_sandboxcharacter_asset_result.txt'
lines = []
lines.append(f'does_asset_exist={unreal.EditorAssetLibrary.does_asset_exist(asset_path)}')
asset = unreal.EditorAssetLibrary.load_asset(asset_path)
lines.append(f'asset={asset}')
if asset:
    lines.append(f'asset_class={asset.get_class().get_name()}')
    lines.append(f'asset_path_name={asset.get_path_name()}')
with open(result_path, 'w', encoding='utf-8') as f:
    f.write('\n'.join(lines))
print('wrote result to', result_path)
