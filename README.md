# HxFLAC

<p align="center">  
    <img src="https://badgen.net/badge/license/MIT/green" />
</p>

Haxe support for the [FLAC](https://xiph.org/flac/) audio format.

This library works on the C++ target only!

Tested on Windows only!

*Inspired by [hxopus](https://github.com/Vortex2Oblivion/hxopus)*

## Installation

```bash
haxelib install hxflac
```

Or through git.

```bash
haxelib git hxflac https://github.com/GreenColdTea/hxflac.git
```

### Usage

Hxflac seamlessly integrates with some popular Haxe game frameworks through easy-to-use helper classes:

### OpenFL

```hx
// Passing the filename in directly
var sound:Sound = FLACHelper.toOpenFLFromFile("path/to/your/sound.flac");
sound.play();

// Or with the raw bytes
var sound:Sound = FLACHelper.toOpenFL(Assets.getBytes("path/to/your/sound.flac"));
sound.play();

// Get metadata from file
var metadata:FLACMetadata = FLACHelper.getMetadataFromFile("path/to/your/sound.flac");
trace('Artist: ${metadata.artist}, Title: ${metadata.title}');
```

### Flixel

```hx
// Passing the filename in directly
var sound:FlxSound = FLACHelper.toFlxSoundFromFile("path/to/your/sound.flac");
sound.play();

// Or with the raw bytes
var sound:FlxSound = FLACHelper.toFlxSound(Assets.getBytes("path/to/your/sound.flac"));
sound.play();

// Get sound with metadata
var result = FLACHelper.toFlxSoundWithMetadata("path/to/your/sound.flac");
trace('Playing: ${result.metadata.artist} - ${result.metadata.title}');
result.sound.play();
```

### Metadata Extraction

Hxflac supports reading FLAC metadata including:

Title, Artist, Album
Genre, Year, Track Number
Comments and etc.

```hx
var metadata = FLACHelper.getMetadataFromFile("path/to/your/sound.flac");
if (metadata != null) {
    trace('Title: ${metadata.title}');
    trace('Artist: ${metadata.artist}');
    trace('Album: ${metadata.album}');
    trace('Genre: ${metadata.genre}');
    trace('Year: ${metadata.year}');
    trace('Track: ${metadata.track}');
    trace('Comment: ${metadata.comment}');
}
```

### Other

If you're using another framework, you can use the low-level decoding functions:

```hx
// Get decoded PCM bytes
var decodedBytes = FLACHelper.decodeFLAC(bytes);

// Get FLAC version
var version = FLACHelper.getVersionString();
```

## Format Support

| Format | Status | Output |
|--------|--------|--------|
| **16-bit FLAC** | ✅ | 16-bit PCM |
| **24-bit FLAC** | ✅ | 24-bit PCM |

### Credits

[Xiph.org Foundation](https://xiph.org) - [LibFLAC](https://github.com/xiph/flac).

[Vortex2Oblivion](https://github.com/Vortex2Oblivion) - [hxopus](https://github.com/Vortex2Oblivion/hxopus) - Initial idea for creating this library.

[NoCopyrightSounds](https://www.youtube.com/@NoCopyrightSounds) - [waera - harinezumi](https://youtu.be/lZ1fj36B42g?si=lYKjX_NbpxPaptxS) Song used for testing this library.
