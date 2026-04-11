import unreal

def inspect_montage(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        unreal.log_error(f"Cannot load {path}")
        return
    
    unreal.log(f"--- Inspecting {path} ---")
    try:
        slot_tracks = asset.get_editor_property('slot_anim_tracks')
        for t in slot_tracks:
            slot_name = t.get_editor_property('slot_name')
            unreal.log(f"  Slot: {slot_name}")
            anim_track = t.get_editor_property('anim_track')
            anim_segments = anim_track.get_editor_property('anim_segments')
            for seg in anim_segments:
                anim_ref = seg.get_editor_property('anim_reference')
                if anim_ref:
                    unreal.log(f"  Spawns Anim: {anim_ref.get_name()}")
    except Exception as e:
        unreal.log_error(f"  Error: {str(e)}")

inspect_montage("/Game/Combo_Attack_01_01_Seq_Montage_Patchouli.Combo_Attack_01_01_Seq_Montage_Patchouli")
inspect_montage("/Game/boss_anim_retarget/AM_Perilous_Slash_Patchouli.AM_Perilous_Slash_Patchouli")
inspect_montage("/Game/boss_anim_retarget/AM_Perilous_Thrust_Patchouli.AM_Perilous_Thrust_Patchouli")
