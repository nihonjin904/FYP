import unreal

# Load the BP_SekiroEnemy blueprint
bp_path = "/Game/BP_SekiroEnemy.BP_SekiroEnemy"
bp = unreal.load_asset(bp_path)
if not bp:
    unreal.log_error("Failed to load BP_SekiroEnemy!")
else:
    # Get the CDO (Class Default Object)
    cdo = unreal.get_default_object(bp.generated_class())
    if not cdo:
        unreal.log_error("Failed to get CDO!")
    else:
        # Find the OverheadWidget component on the CDO
        components = cdo.get_components_by_class(unreal.WidgetComponent)
        unreal.log(f"Found {len(components)} WidgetComponents on CDO")
        
        for comp in components:
            unreal.log(f"  Component: {comp.get_name()}, WidgetClass: {comp.get_editor_property('widget_class')}")
            
            if "Overhead" in comp.get_name():
                # Load WBP_OVERHEAD widget class
                widget_bp = unreal.load_asset("/Game/WBP_OVERHEAD.WBP_OVERHEAD")
                if widget_bp:
                    widget_class = widget_bp.generated_class()
                    comp.set_editor_property("widget_class", widget_class)
                    comp.set_editor_property("widget_space", unreal.WidgetSpace.SCREEN)
                    comp.set_editor_property("draw_at_desired_size", True)
                    unreal.log(f"  -> Set WidgetClass to WBP_OVERHEAD on {comp.get_name()}")
                else:
                    unreal.log_error("Failed to load WBP_OVERHEAD!")
        
        # Save the blueprint
        unreal.EditorAssetLibrary.save_asset(bp_path)
        unreal.log("BP_SekiroEnemy saved!")

# Also check the placed actor in the level
actors = unreal.GameplayStatics.get_all_actors_of_class(unreal.EditorLevelLibrary.get_editor_world(), unreal.type(cdo))
unreal.log(f"Found {len(actors) if actors else 0} actors of this class in level")
