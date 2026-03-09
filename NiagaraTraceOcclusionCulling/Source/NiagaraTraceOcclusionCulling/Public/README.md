### Dynamic Niagara Optimization: Trace-Occlusion Culling 
I designed a runtime system which "culls" Niagara instances not visible to the player.  "Culling," in this case, is pausing systems and setting visibility to false.  I check for occlusion through efficient batching of a fixed (small) number of line traces per tick, and use a manageably low tick rate.

This solves a problem ignored by the base Niagara framework:  Niagara systems simulate even when the player can't see them. In large games, unneeded simulation can eat up substantial CPU and GPU time.  Also, occluded systems still incur emitter update costs, and have a (minor) impact on the frustum culling loop.

The use case for this optimization is games with lots of VFX, and lots of occluded sightlines.  It was developed for use in a first-person game set in a dense city. Lots of buildings, boxes, vehicles, etc. to occlude the player's sightline (and provide cover).  And lots and lots of VFX to provide life.  In this real-world use case, I was able to shave a significant amount of Niagara overhead.

[Demo Video](https://youtu.be/zfOdDgG9BEM)

The display shows both Niagara stats, and the overhead of the system itself.  There's very little overhead required for a sizable optimization.
