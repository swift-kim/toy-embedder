# Flutter on Tizen

A minimal Flutter embedder implementation for Tizen inspired by [flutter_from_scratch](https://github.com/chinmaygarde/flutter_from_scratch).

Only the following features are supported:

- Display a Flutter application on the screen
- Process touch inputs

## How to use

1. First build the Flutter engine by following [this guide](https://gist.github.com/swift-kim/f2eb9d59695188ea5ee5ea391ce38a0a). The same toolchain as used for building the engine is used for building the embedder.

2. Replace `libflutter_engine.so`, `icudtl.dat` and `flutter_embedder.h` files in the `out` directory with what you've just built.

3. Open the `BUILD.gn` file and update `toolchain_path` and `sysroot_path` values if required.

4. Run `./gn gen out`.

5. Run `ninja -C out`. If successful, `libflutter_embedder.so` is generated in the `out` directory.

6. (TBD) Using [`flutter_tizen.h`](src/public/flutter_tizen.h) and the Tizen SDK, create an application wrapper to run a Flutter application.
