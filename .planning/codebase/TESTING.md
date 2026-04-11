# Testing Practices

## Framework
There is no dedicated Automation Testing framework (like Unreal Automation System or Gauntlet) setup in the traditional CI/CD sense. 

## Python-Driven Diagnostics
Validation relies on custom Python diagnostic scripts run proactively inside the Unreal Editor:
1. `diagnose_notifies.py`
2. `diagnose_bp.py`
3. `diagnose_weapon.py`

These scripts validate properties (ensuring correct IK roots, collision configurations on weapon components, and proper Montage mappings) instead of typical gameplay runtime unit tests.
