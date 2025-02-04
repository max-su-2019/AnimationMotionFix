# Introduction Of Animation Motion Fix 

## Synopsis
**Animation Motion Fix** (refer as AMF) is a SKSE plugin that intend to fix issues of Skyrim game engine 's animation root motion system. In vanilla Skyrim, actors would encounter motion reduction or motion stuck issue when they performing root motion animation during combat, while those issues just getting much more apparent and unbearable in modern combat animations mods environment. With AMF installed, actors motion would no longer get reduced or stuck when they performing root motion animation.

## Mod Features

### Fix - Fix NPC pitch angle translation 
The vanilla game engine has used a wrong method to compute NPCs animation motion when NPCs were on ground(not flying), it has taken consideration of the NPCs pitch angle offset and transferred the displacement amount from Y axis to Z axis, thus NPCs Y axis displacement would got reduced in some degree that proportional with their pitch angle offset.
![image1](./Slide1.png)
![image2](./Slide2.png)

This issue usually occurs when NPCs are in combat since NPCs always adjusts their pitch angle during combat.
One of the most common case in vanilla is NPCs stagger animations would be markedly reduced after entered combat state.  
To enable this fix, you need to have setting `EnablePitchTranslationFix` value be true in "AnimationMotionFix.ini".
 ```ini
 [Fix]
 EnablePitchTranslationFix = true

 ```  
<br>

 ### Tweak - Disable Player Rotation Magnetism effect
The vanilla game engine has applied some kind of rotation magnetism effect for player character in combat, it would continually adjust the player rotation motion value each frame in order to trace an enemy, It might not works well with those modern action combat mods, especially with TDM installed. To disable this effect, you need to have setting `DisablePlayerRotationMagnetism` value be true in "AnimationMotionFix.ini".
```ini
[Tweak]
DisablePlayerRotationMagnetism = true
```
<br>  

 ### Tweak - Disable Attack Movement Magnetism effect
The vanilla game engine has applied some kind of movement magnetism affect on an attack animation motion driven actor which would caused the actors animation root motion value be cleared to zero when it collided with the combat target. As a result, the actors attack animations would lost displacement and get stuck in place after collided with another actor in combat. It might be all right for the vanilla attack animations, but might not works well when combining with those modern action combat mods.
To disable actors attack magnetism effect and prevent motion stuck situation, you need to have setting `DisablePlayerMovementMagnetism` & `DisableNpcMovementMagnetism` value be true in "AnimationMotionFix.ini".
```ini
[Tweak]
DisablePlayerMovementMagnetism = true
DisableNpcMovementMagnetism = true
```

## Requirements
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)

