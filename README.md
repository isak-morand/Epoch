# Epoch

Epcoh is a game engine I'm working on during my free time. The goal is to learn as much as possible and grow as a programmer, but also to be able to create and maybe even publish some small games.
My main source of inspiration and resource is [Hazel](https://hazelengine.com/) by **Studio Cherno**.

***

## Getting Started

For now, Windows is the only supported platform.

<ins>**1. Downloading the repository:**</ins>

The repository can either be downloaded directly or cloned. When cloning, recursive cloning isn't needed as no submodules are currently used.
This will probably be changed in the future to minimize the repositories size.

<ins>**2. Generate project:**</ins>

Running [Script/Generate.bat](https://github.com/isak-morand/Epoch/blob/main/Generate.bat) will generate the solution files.

<ins>**2. Building and running:**</ins>

Before you build you should know about the configurations. `Debug` runs slow and is only used for debugging. `Release` is the main config used for development. `Dist` is used to build distribution builds that run without the console, not used for development except for testing purposes.

Before we can build and run, we need to build the Epoch-ScriptCore manually. To do so, right-click `Solution Explorer -> Core -> Epoch-ScriptCore` and press `Build`.

You can now build and run the engine/editor.
