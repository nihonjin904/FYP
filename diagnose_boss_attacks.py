import unreal

def log(msg):
    unreal.log_warning("[Diagnose AI] " + str(msg))

def diagnose_boss():
    log("=== STARTING BOSS AI DIAGNOSIS ===")
    
    boss_bp_path = "/Game/BP_SekiroEnemy" # Changed from BP_SekiroEnemy_Boss based on search
    boss_bp = unreal.load_asset(boss_bp_path)
    
    if not boss_bp:
        log("ERROR: Could not find " + boss_bp_path)
        return
        
    cdo = unreal.get_default_object(boss_bp.generated_class())
    
    # 1. Check AI Controller mapping
    try:
        log("AIControllerClass: " + str(cdo.get_editor_property("AIControllerClass")))
    except: pass
    
    # 2. Dump all blueprint properties looking for Montages
    log("\n--- DUMPING POTENTIAL ATTACK VARIABLES ---")
    prop_names = dir(cdo)
    found_any = False
    for p in prop_names:
        pl = p.lower()
        if "montage" in pl or "attack" in pl or "action" in pl or "combo" in pl or "skill" in pl or "chance" in pl:
            try:
                val = cdo.get_editor_property(p)
                log(f"{p}: {val}")
                found_any = True
            except:
                pass
                
    if not found_any:
        log("No explicit attack arrays found in CDO root. Logic might be inside the Behavior Tree or C++ CombatComponent.")
        
    # 3. Check Behavior Trees globally
    asset_reg = unreal.AssetRegistryHelpers.get_asset_registry()
    bt_assets = asset_reg.get_assets_by_class("BehaviorTree")
    log("\n--- FOUND BEHAVIOR TREES IN PROJECT ---")
    for bt in bt_assets:
        log(str(bt.package_name))

    log("=== END DIAGNOSIS ===")

if __name__ == "__main__":
    diagnose_boss()
