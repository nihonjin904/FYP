# Technical Concerns and Debt

## Python Hardcoded Asset Paths
The Python automation scripts (e.g. `batch_retarget_reimu.py`) rely extensively on hardcoded internal paths (`/Game/Characters/...`). Any directory shift in Editor will spontaneously break the automation pipeline.

## UI Binding Reliability
There's an implicit architectural gap between Screen Space Widgets, World Space Overheads, and Owner replication. The recent implementation mitigates this by manually applying `BindToActor`, but care must be taken that dynamically spawned logic explicitly calls this whenever the visual hierarchy omits implicit bounds.

## C++ vs Blueprint Overrides
Constructor defaults in C++ are sometimes overridden locally or inside CDOs (Class Default Objects) dynamically via Python manipulation (`set_editor_property`). It can become obscure to trace down whether a value is set natively in C++, inside the Blueprint Editor GUI, or injected via the auto-repair Python scripts.
