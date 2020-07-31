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
using System.Collections.Generic;
using System.IO;
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

        [DllImport("flutter_embedder.so")]
        internal static extern IntPtr RunFlutterApplication(
            IntPtr /*FlutterDesktopSize*/ size,
            IntPtr /*FlutterDesktopEngineProperties*/ engine_properties,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] string[] switches,
            uint switches_count);

        [DllImport("flutter_embedder.so")]
        internal static extern IntPtr StopFlutterApplication(IntPtr /*FlutterApplicationRef*/ application);
        #endregion

        #region flutter_embedder.h
        [DllImport("flutter_engine.so")]
        internal static extern bool FlutterEngineRunsAOTCompiledDartCode();
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

            var pSize = Marshal.AllocHGlobal(Marshal.SizeOf(size));
            var pProperties = Marshal.AllocHGlobal(Marshal.SizeOf(properties));
            Marshal.StructureToPtr<FlutterDesktopSize>(size, pSize, false);
            Marshal.StructureToPtr<FlutterDesktopEngineProperties>(properties, pProperties, false);

            try
            {
                Instance = RunFlutterApplication(pSize, pProperties, args.ToArray(), (uint)args.Count);
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("Could not start the Flutter application: " + ex);
            }
            finally
            {
                Marshal.FreeHGlobal(pSize);
                Marshal.FreeHGlobal(pProperties);
            }
        }

        protected override void OnTerminate()
        {
            base.OnTerminate();

            try
            {
                if (Instance != IntPtr.Zero)
                {
                    StopFlutterApplication(Instance);
                }
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("Could not stop the Flutter application: " + ex);
            }
        }

        static void Main(string[] args)
        {
            App app = new App();
            app.Run(args);
        }
    }
}
