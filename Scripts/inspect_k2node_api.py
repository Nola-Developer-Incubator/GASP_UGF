import unreal

for cls_name in ['K2Node_CallFunction', 'K2Node_VariableGet', 'K2Node_VariableGet', 'K2Node_FunctionEntry', 'K2Node_GetVariableByName']:
    cls = getattr(unreal, cls_name, None)
    print('\nClass:', cls_name, 'exists=', cls is not None)
    if cls is not None:
        attrs = [a for a in dir(cls) if 'get' in a.lower() or 'pin' in a.lower() or 'node' in a.lower() or 'function' in a.lower()]
        print('  attrs:', attrs[:80])
        print('  len(attrs)=', len(attrs))
