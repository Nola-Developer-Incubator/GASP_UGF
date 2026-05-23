import unreal

asset_path = '/Game/Blueprints/SandboxCharacter_GASP'
blueprint = unreal.EditorAssetLibrary.load_asset(asset_path)
if not blueprint:
    print(f'Failed to load asset: {asset_path}')
    raise SystemExit(1)

print('Blueprint class:', blueprint.get_class().get_name())
print('Blueprint attributes containing graph:')
for attr in dir(blueprint):
    if 'graph' in attr.lower() or 'uber' in attr.lower():
        print('  ', attr)

print('\nBlueprint class methods with graph names:')
for attr in dir(blueprint):
    if callable(getattr(blueprint, attr)) and 'graph' in attr.lower():
        print('  ', attr)

if hasattr(blueprint, 'get_editor_property'):
    for prop in ['uber_graph_pages', 'function_graphs', 'delegate_signature_graphs', 'simple_construction_script', 'event_graph']:
        if hasattr(blueprint, prop):
            value = getattr(blueprint, prop)
            print(f'Property {prop}: type={type(value)}')
            try:
                print('  len=', len(value))
            except Exception as e:
                print('  len error', e)
