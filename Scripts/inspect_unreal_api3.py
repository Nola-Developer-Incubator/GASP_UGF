import unreal

def log(msg):
    unreal.log(msg)

log('--- inspect_unreal_api3 start ---')
log('has BlueprintEditorLibrary: %s' % hasattr(unreal, 'BlueprintEditorLibrary'))
log('has KismetEditorUtilities: %s' % hasattr(unreal, 'KismetEditorUtilities'))
log('has EditorAssetLibrary: %s' % hasattr(unreal, 'EditorAssetLibrary'))
log('has K2Node: %s' % hasattr(unreal, 'K2Node'))
if hasattr(unreal, 'BlueprintEditorLibrary'):
    libs = [m for m in dir(unreal.BlueprintEditorLibrary) if 'graph' in m.lower() or 'node' in m.lower() or 'compile' in m.lower() or 'property' in m.lower()]
    log('BlueprintEditorLibrary relevant members: %s' % libs)
if hasattr(unreal, 'KismetEditorUtilities'):
    libs = [m for m in dir(unreal.KismetEditorUtilities) if 'graph' in m.lower() or 'node' in m.lower() or 'blueprint' in m.lower()]
    log('KismetEditorUtilities relevant members: %s' % libs)
if hasattr(unreal, 'K2Node'):
    libs = [m for m in dir(unreal.K2Node) if 'get' in m.lower() or 'pin' in m.lower() or 'graph' in m.lower()]
    log('K2Node relevant members sample: %s' % libs[:50])
log('--- inspect_unreal_api3 end ---')
