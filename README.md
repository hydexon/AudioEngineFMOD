<div align="center">
  <img src="fmod_logo.png">
</div>

**WARNING**: This Gem is due for a rewrite, we don't recommend using this gem just now.

# FMOD Studio for O3DE AudioSystem.

This gem provides a _(non-functional)_ AudioSystem implementation for O3DE using FMOD Studio, this gem also aims to be simpler in their asset pipelines
for soundbanks and individual audio streaming files, removing the need to generate settings, special dependency files generated through
scripts separatedly, as well to be fully multiplatform in both runtime and authoring.

**NOTE:**: Users wil need to download FMOD Engine SDK from [FMOD website](https://www.fmod.com/download), extract and set the FMOD SDK root folder
in CMake under the `FMOD_ROOT` variable, or set an environment variable called `FMOD_HOME` in your system.

## Project Info generator script to FMOD Studio.
In order to maintain your O3DE project clean, you don't need to copy your FMOD project within O3DE, in order to do this, you need to install a small tool script located `Scripts/generateO3DEProjectInfo.js` to your installation of FMOD Studio, you have two options: project-based or installation based.

Both options enables FMOD Studio generate the required JSON file called *"FMODProjectInfo.json"* needed by the Audio Control Editor within the O3DE Editor with the the AudioEngineFMOD gem enabled, the JSON generation is triggered manually by clicking **Scripts > O3DE FMOD AudioSystem > Generate FMOD Project Info** in FMOD Studio Main Menu, will generate an JSON file in your project directory, and building banks into O3DE Assets/Audio/FMOD/Banks, will generate the file into Assets/Audio/FMOD where the editor will look for.

### Installing script to your FMOD Project
Just simply copy the script to your project's Scripts directory (i.e `%project_root_directory%/Scripts`), create the "Scripts" folder if needed

### Installing script to your FMOD Studio install

Copy the script in the following locations according your platform:
* **Windows**: `%localappdata%/FMOD Studio/Scripts`
* **Mac**: `~/Library/Preferences/FMOD Studio/Scripts`
* **Linux**: `~/.config/fmod-studio/Scripts`

