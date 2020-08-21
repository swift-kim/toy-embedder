/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

using System;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Tizen.Applications;
using Tizen.System;

namespace FlutterApplication
{
    internal class App : CoreUIApplication
    {
        protected IntPtr /*FlutterApplicationRef*/ Instance = IntPtr.Zero;

        #region flutter_tizen.h
        [StructLayout(LayoutKind.Sequential)]
        internal struct FlutterDisplaySize
        {
            public int width;
            public int height;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct FlutterEngineProperties
        {
            public string assets_path;
            public string icu_data_path;
            public string aot_library_path;
        }

        [DllImport("flutter_embedder.so")]
        internal static extern IntPtr RunFlutterApplication(
            ref FlutterDisplaySize size,
            ref FlutterEngineProperties engine_properties);

        [DllImport("flutter_embedder.so")]
        internal static extern IntPtr StopFlutterApplication(IntPtr application);
        #endregion

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
            var size = new FlutterDisplaySize
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

            var properties = new FlutterEngineProperties
            {
                assets_path = assetsPath,
                icu_data_path = icuDataPath,
                aot_library_path = aotLibPath,
            };

            Instance = RunFlutterApplication(ref Unsafe.AsRef(size), ref Unsafe.AsRef(properties));
            if (Instance == IntPtr.Zero)
            {
                throw new Exception("Could not launch a Flutter application.");
            }
        }

        protected override void OnTerminate()
        {
            base.OnTerminate();

            if (Instance != IntPtr.Zero)
            {
                StopFlutterApplication(Instance);
            }
        }

        static void Main(string[] args)
        {
            App app = new App();
            app.Run(args);
        }
    }
}
