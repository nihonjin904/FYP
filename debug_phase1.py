import unreal

def log(msg):
    unreal.log_warning(f"[DebugPhase1] {msg}")

def run_diagnostics():
    log("=== STARTING DIAGNOSTICS FOR BOSS & UI ===")
    
    # 1. Check Blueprints
    assets = unreal.EditorAssetLibrary.list_assets("/Game", recursive=True)
    enemy_bps = [a for a in assets if "SekiroEnemy" in a and "BP_" in a]
    log(f"Found Sekiro Enemy BPs: {enemy_bps}")
    
    # 2. Check WBP_Overhead
    wbp_overhead = [a for a in assets if "WBP_Overhead" in a]
    log(f"Found WBP_Overhead Assets: {wbp_overhead}")
    
    # 3. Check Anim Blueprint
    abp_new = [a for a in assets if "ABP_SekiroEnemy_New" in a]
    log(f"Found ABP_SekiroEnemy_New: {abp_new}")
    
    # 4. Check Anim Sequences and Montages loaded in /Game/boss_anim_retarget
    boss_anims = unreal.EditorAssetLibrary.list_assets("/Game/boss_anim_retarget", recursive=True)
    log(f"Assets in /Game/boss_anim_retarget: {boss_anims}")
    
    boss_anims_orig = unreal.EditorAssetLibrary.list_assets("/Game/boss_animation", recursive=True)
    log(f"Assets in /Game/boss_animation: {boss_anims_orig}")

    log("=== DIAGNOSTICS COMPLETED ===")

if __name__ == "__main__":
    run_diagnostics()
