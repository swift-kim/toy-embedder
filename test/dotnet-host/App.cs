using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using Tizen.Applications;
using Tizen.System;

namespace FlutterApplication
{
    internal class App : CoreUIApplication
    {
        [StructLayout(LayoutKind.Sequential)]
        internal struct FlutterDesktopSize
        {
            public int width;
            public int height;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct FlutterDesktopEngineProperties
        {
            public string assets_path;
            public string icu_data_path;
        }

        // libflutter_engine.so is needed by libflutter_embedder.so.
        // By default, the loader cannot locate libflutter_engine.so because it's not in LD_LIBRARY_PATH.
        // A call to libflutter_engine.so must be made to make sure the library is loaded by CLR.
        [DllImport("flutter_engine.so")]
        internal static extern bool FlutterEngineRunsAOTCompiledDartCode();

        // There's no clear way to marshal a struct containing a string array into a C struct.
        // Therefore, we directly pass an array of switches to the embedder without wrapping it as a struct.
        [DllImport("flutter_embedder.so")]
        internal static extern IntPtr RunFlutterApplication(
            FlutterDesktopSize size,
            FlutterDesktopEngineProperties engine_properties,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] string[] switches,
            uint switches_count);

        protected override void OnCreate()
        {
            base.OnCreate();

            Console.WriteLine("Preparing to start a Flutter application...");

            if (!Information.TryGetValue("http://tizen.org/feature/screen.width", out int width) ||
                !Information.TryGetValue("http://tizen.org/feature/screen.height", out int height))
            {
                Console.Error.WriteLine("Could not obtain the screen size.");
                return;
            }
            var size = new FlutterDesktopSize
            {
                width = width,
                height = height
            };

            var appRoot = Path.GetDirectoryName(Path.GetDirectoryName(Current.ApplicationInfo.ExecutablePath));
            var arch = RuntimeInformation.ProcessArchitecture switch
            {
                Architecture.X86 => "x86",
                Architecture.X64 => "x64",
                Architecture.Arm => "arm",
                Architecture.Arm64 => "aarch64",
                _ => "",
            };
            var aotLibPath = Path.Combine(appRoot, "lib", arch, "libapp.so");
            var assetsPath = Path.Combine(appRoot, "res", "flutter_assets");
            var icuDataPath = Path.Combine(appRoot, "res", "icudtl.dat");

            var args = new List<string>
            {
                "--verbose-logging",
                "--trace-startup",
                "--disable-service-auth-codes",
            };

            if (FlutterEngineRunsAOTCompiledDartCode())
            {
                Console.WriteLine("Run AOT compiled Dart code: " + aotLibPath);
                args.Add("--aot-shared-library-name=" + aotLibPath);
            }

            var properties = new FlutterDesktopEngineProperties
            {
                assets_path = assetsPath,
                icu_data_path = icuDataPath,
            };

            _ = RunFlutterApplication(size, properties, args.ToArray(), (uint)args.Count);
        }

        static void Main(string[] args)
        {
            App app = new App();
            app.Run(args);
        }
    }
}
