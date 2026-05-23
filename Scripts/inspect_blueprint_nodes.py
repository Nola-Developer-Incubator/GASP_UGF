import unreal

asset_path = '/Game/Blueprints/SandboxCharacter_GASP'
blueprint = unreal.EditorAssetLibrary.load_asset(asset_path)
if not blueprint:
    print(f'Failed to load asset: {asset_path}')
    raise SystemExit(1)

print(f'Loaded blueprint: {asset_path}')

all_graphs = blueprint.get_all_graphs()
print(f'Found {len(all_graphs)} graphs')
for graph in all_graphs:
    print(f'Graph: {graph.get_name()} ({graph.get_class().get_name()})')
    for node in graph.get_nodes():
        node_class = node.get_class().get_name()
        # Filter mostly call/function nodes
        if 'Call' in node_class or 'Function' in node_class or 'K2Node' in node_class:
            pins = node.get_pins()
            for pin in pins:
                if pin.get_name() == 'Target' and not pin.is_linked():
                    print(f'  Node: {node.get_name()} class={node_class} title={node.get_node_title(None)}')
                    print(f'    target pin not linked; type={pin.get_pin_type().pin_category} name={pin.get_pin_type().pin_sub_category_object}')
                    # maybe print function path from node properties
                    if hasattr(node, 'get_function_name'):
                        print(f'    function: {node.get_function_name()}')
                    if hasattr(node, 'get_target_pin'):
                        print(f'    target pin caption: {node.get_target_pin().get_name()}')
