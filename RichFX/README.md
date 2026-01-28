# RichFX System
Unreal system which expands the concept of "effects" to include more than just Niagara systems.  It groups together and manages all effect responses to specific gameplay events.  Configuration is via data tables.

This patches a hole in the GameplayAbilitySystem: effect responses to events are often multi-layered entities, not just Niagara systems.  In a complex game with a very large asset base, the connection between effect assets in different categories (Niagara, decals, post, sound, rumble, camera shake, etc.) is often lost, which limits artists' ability to maintain "rich" responses to events.  For example, a combat hit might trigger a non-digetic player cue, a digetic blood splat, a blood decal on the ground, an audio effect, and controller rumble.  

In this system, all those pieces are bundled and sequenced.  And, they are easily managed by the data table.

The system also includes object pooling and some basic runtime performance management features.
