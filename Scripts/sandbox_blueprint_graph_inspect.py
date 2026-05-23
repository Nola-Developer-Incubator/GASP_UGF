import unreal
import pathlib

output_file = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/sandbox_blueprint_graph_inspect.txt')
asset_path = '/Game/Blueprints/SandboxCharacter_GASP'
blueprint = unreal.BlueprintEditorLibrary.get_blueprint_asset(asset_path)
with output_file.open('w', encoding='utf-8') as f:
    f.write('blueprint class: %s\n' % (blueprint.get_class().get_name() if blueprint else 'NONE'))
    if not blueprint:
        raise SystemExit(1)
    try:
        uber = blueprint.get_editor_property('uber_graph_pages')
        f.write('uber_graph_pages count: %s\n' % (len(uber) if uber is not None else 'NONE'))
        for graph in uber or []:
            f.write('graph: %s class=%s\n' % (graph.get_name(), graph.get_class().get_name()))
            if hasattr(graph, 'get_nodes'):
                nodes = graph.get_nodes()
                f.write('  node count=%s\n' % len(nodes))
                # write first 80 nodes and their class names, target pin names if possible
                for node in nodes[:80]:
                    f.write('  node %s class=%s name=%s\n' % (node.get_name(), node.get_class().get_name(), node.get_node_title(unreal.EditorStyleLibrary.get_editor_style())) if hasattr(node, 'get_node_title') else ('  node %s class=%s\n' % (node.get_name(), node.get_class().get_name())))
            else:
                f.write('  graph has no get_nodes\n')
    except Exception as e:
        f.write('Exception listing uber_graph_pages: %s\n' % e)

    # try find event graph and list some nodes
    try:
        event_graph = unreal.BlueprintEditorLibrary.find_event_graph(blueprint)
        f.write('event_graph: %s class=%s\n' % (event_graph.get_name(), event_graph.get_class().get_name()))
        nodes = event_graph.get_nodes() if hasattr(event_graph, 'get_nodes') else []
        f.write('event graph node count=%s\n' % len(nodes))
    except Exception as e:
        f.write('find_event_graph exception: %s\n' % e)

    f.write('done\n')
print('wrote', output_file)
