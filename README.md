![BLENNY!](https://frank2.github.io/img/blenny.png "BLENNY!")

**Blenny** is a proof-of-concept payload obfuscation tool for Windows which hides executable payload data within the icon of the executable. It is named after the [sabre-toothed blenny fish](https://en.wikipedia.org/wiki/Sabre-toothed_blenny), which mimmicks the behavior and looks of cleaner fish to prey on unsuspecting fish.

## Why?

I can't recall exactly how the story goes, but a hacker named retr0id ([@DavidBuchanan314](https://github.com/DavidBuchanan314)) brought up the idea of hiding a payload within the icon of an executable. Because of various quirks of PNG files-- and because Windows icon files can have PNG data within them-- I realized I could implement this idea pretty easily, it would just require a bit of elbow grease. I created [the Facade library](https://github.com/frank2/facade)-- a library with the specific purpose of generating payloads in PNG files-- for this very purpose.

The eventual result is a compact payload file and build system which can be used to customize a basic dropper/executer payload for Windows.

## Acquiring

**Blenny** makes use of [the Facade library](https://github.com/frank2/facade) for creating payloads out of icon files, so when grabbing the repository, the submodule (and its submodules) need to be initialized:

```
$ git clone https://github.com/frank2/blenny.git
$ cd blenny
$ git submodule update --init --recursive
```

This should be all you need to initialize the repository for building.

## Building

This project was literally designed with CMake in mind, so before building this project, it is an absolute requirement you [acquire and install CMake](https://cmake.org). It will probably not work otherwise (unless you're *really* dedicated to manually setting all the CMake definitions yourself). You will also need a copy of [Visual Studio](https://visualstudio.microsoft.com) to compile the project.

This project makes use of numerous CMake variables to configure the build process. To get a basic demonstration payload, in which a lovely sheep friend will land on your desktop, you can run the following in the main directory of the project:

```
$ mkdir build
$ cd build
$ cmake -D BLENNY_ZTEXT_PAYLOAD=ON ../
$ cmake --build ./ --config Release
$ ./Release/blenny.exe
```

By default, this extracts an executable named "blenny.exe" from the executable's icon into the current user's temp directory, then executes it. The default payload is [@AdrianoTiger](https://github.com/adrianotiger)'s [DesktopPet](https://github.com/Adrianotiger/desktopPet/), a dedicated rebuild to the classic esheep.exe from the 90s.

All the variables that make this happen are customizable! One payload variable (BLENNY\_TRAILING\_PAYLOAD, BLENNY\_TEXT\_PAYLOAD, BLENNY\_ZTEXT\_PAYLOAD or BLENNY\_STEGO\_PAYLOAD) must be set. This controls how the payload is eventually stuffed in the icon. See [the Facade library](https://github.com/frank2/facade) for details on these techniques. For example, to use your own payload, do this:

```
$ cmake -D BLENNY_PAYLOAD_FILE="path/to/your/payload.exe" -D BLENNY_ZTEXT_PAYLOAD=ON ../
$ cmake --build ./ --config Release
```

Here's a table of all the different configuration options you can set, their default values, and what they do.

| Flag | Type | Default value | Purpose |
| ---- | ---- | ------------- | ------- |
| BLENNY\_PAYLOAD\_FILE | String | `"${PROJECT_SOURCE_DIR}/res/defaultpayload.exe"` | The payload to embed in the icon file. **Note:** Because the eventual execution technique is `ShellExecuteA`, this can be any file that can be launched from Explorer, such as a batch script! |
| BLENNY\_ICON\_FILE | String | `"${PROJECT_SOURCE_DIR}/res/defaulticon.ico"` | The icon to use for the executable and embed the payload in. **Note:** Not all icons will work! A PNG section is required to be in the icon file. Check your target icon's data in a hex editor to see if a PNG image is included! |
| BLENNY\_TRAILING\_PAYLOAD | Boolean | `OFF` | Enable a trailing data payload within the icon's PNG file. |
| BLENNY\_TEXT\_PAYLOAD | Boolean | `OFF` | Enable a `tEXt` section payload within the icon's PNG file. |
| BLENNY\_ZTEXT\_PAYLOAD | Boolean | `OFF` | Enable a `zTXt` section payload within the icon's PNG file, which is a compressed `tEXt` payload. |
| BLENNY\_STEGO\_PAYLOAD | Boolean | `OFF` | Enable a steganographic payload within the icon's PNG file. **Note:** because of the image size of most icons being limited to 256x256, not all payloads will fit in this option! |
| BLENNY\_PAYLOAD\_KEYWORD | String | `"blenny"` | For text/ztext payloads, set the keyword labelling the payload data. |
| BLENNY\_ADMIN | Boolean | `OFF` | Enables the `runas` keyword to run the payload as an administrator. |
| BLENNY\_PATH | String | `"%TEMP%"` | The path to store the payload before executing. Environment variables are expanded at execution time. |
| BLENNY\_FILENAME | String | `"blenny"` | The filename to give the payload upon extraction. |
| BLENNY\_RANDOM\_FILENAME | Boolean | `OFF` | Enable a random filename to be generated for the payload upon extraction. |
| BLENNY\_RANDOM\_FILENAME\_LENGTH | Integer | `8` | Adjust the length of the random filename. |
| BLENNY\_RANDOM\_FILENAME\_RANDOM\_LENGTH | Boolean | `OFF` | Give the filename a random length upon extraction. |
| BLENNY\_PAYLOAD\_ARGS | String | `""` | The arguments to give the payload file upon execution. |
| BLENNY\_FILE\_ATTRIBUTES | Integer | `0x80` (aka `FILE_ATTRIBUTE_NORMAL`) | The file attributes to give to the file upon creation. |
| BLENNY\_CMD\_SHOW | Integer | `5` (aka `SW_SHOW`) | The visual disposition of the payload file upon execution. Another example is `SW_HIDE`, which is 0. |
