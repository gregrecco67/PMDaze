
# PM Daze

An expressive, semi-modular *phase-modulation* synthesizer. Cousin(?) of this [other synth](https://github.com/gregrecco67/AudiblePlanets).

## Installation
The plugin is available in a variety of formats [here](https://github.com/gregrecco67/PMDaze/releases). Since there is no installer, you will have to place the plugin file in the right place yourself. On Windows, place the VST3 file in /Program Files/Common Files/VST3. On Mac OS, place the VST3 file in ~/Library/Audio/Plug-Ins/VST3 and/or the AU file in \~/Library/Audio/Plug-Ins/Components. (You will need the Audio Unit component if you plan to run the synth in Logic or GarageBand.) On Linux, place the VST3 file wherever your DAW looks for it, which should include ~/.vst3 as a default. More info on default VST3 file locations [here](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical+Documentation/Locations+Format/Plugin+Locations.html) and LV2 locations [here](https://lv2plug.in/pages/filesystem-hierarchy-standard.html). On a Mac, you will need to take the further step of authorizing the plugin to run, either from the Privacy & Security settings panel, or by typing `xattr -dr com.apple.quarantine <file location>` in a Terminal window.

Alternatively, you can build it from source:
```
git clone --recurse-submodules https://github.com/gregrecco67/PMDaze.git
cd PMDaze 
```
Then, if on Mac OS:
```
cmake -B build -G Xcode .
open build/PMDaze.xcodeproj
```
and compile the project in Xcode.

Or, on [Linux](https://github.com/juce-framework/JUCE/blob/master/docs/Linux%20Dependencies.md) (using Ninja):
```
cmake -B ninja-build -DCMAKE_BUILD_TYPE=Release -G Ninja .
cd ninja-build
ninja -j6
```

On Windows, using Visual Studio, just open the folder and wait for it to parse the CMake file. The project is configured to install the plugin in the right place after building it.

# Operation

## Pages

As seen above, the synth has three pages: "Main," "Mods," and "Effects." The Main page contains the main controls for sound generation. The Mods page contains controls for configurable modulation sources like LFOs and MSEGs. The Effects page contains the built-in effects arranged in one or two chains.

### Main Page

Almost every control on the main page can by modulated differently for each individual voice ("poly") or the same for all ("mono"). A quick aside about that:

<ul>On all three pages, the "Mod Sources" and "Mod Matrix" panels appear. Click on any of the icons to the right of modulation sources to select them for assignment to one or more controls, which can be done by clicking and dragging on any knob. Modulation sources and destinations will appear in the mod matrix, which offers further controls for different modulation curves and amounts and for selecting between unipolar or bipolar modulation. Note that many modulation destinations, and all effects controls, can only be modulated "mono."</ul>

### Mods Page

The "Mods" page contains the controls for the LFOs and MSEGs, as well as a few others. In the headers of most mod source panels (ENVs, LFOs, MSEGs), there are controls to create connections, either by direct selection, or by putting the synth in "Learning" mode, where clicking and dragging controls applies modulation. There are also some overall controls for the synth in "Global." Finally, the "Macros" panels allows modulation of several parameters at once by one knob, which can be assigned to respond to a MIDI CC message. Just click "Learn" beneath the knob, and then activate the control you want to assign to it on your MIDI controller. A CC number will appear below the knob to indicate the assignment was successful. Click on the "Clear" button below it to remove the assignment.

### Effects Page

The "Effects" page contains slots for up to eight built-in effects, arranged in one or two chains ("A -> B" or "A || B"), each with its own gain control, applied before the signal enters the chain. Ordering the effects chains differently can open up a lot of possibilities. Since the effects parameters can be modulation destinations, there's a mod sources panel on this page, too, as well as a modulation matrix panel, showing the currently active modulations, and offering controls to bypass, adjust, or delete them. In combination, the effects on this page can add a lot of boost, especially when modulated with sources that can constructively interfere. A built-in limiter is accordingly always on and meant to keep the levels below 0 dB, but be careful nonetheless.

