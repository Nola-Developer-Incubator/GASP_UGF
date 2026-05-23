import unreal, pathlib
output = pathlib.Path(r'C:/Unreal_Projects/GASP/Scripts/graph_node_api.txt')
with output.open('w', encoding='utf-8') as f:
    node_cls = unreal.EdGraphNode
    f.write('EdGraphNode members:\n')
    for name in sorted(dir(node_cls)):
        if not name.startswith('_'):
            f.write(name + '\n')
    pin_cls = unreal.EdGraphPin
    f.write('\nEdGraphPin members:\n')
    for name in sorted(dir(pin_cls)):
        if not name.startswith('_'):
            f.write(name + '\n')
    graph_cls = unreal.EdGraph
    f.write('\nEdGraph members:\n')
    for name in sorted(dir(graph_cls)):
        if not name.startswith('_'):
            f.write(name + '\n')
print('done')
