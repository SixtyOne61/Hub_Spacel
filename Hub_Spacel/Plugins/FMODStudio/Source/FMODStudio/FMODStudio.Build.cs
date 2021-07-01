// Copyright (c), Firelight Technologies Pty, Ltd. 2012-2021.
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class FMODStudio : ModuleRules
    {
    #if WITH_FORWARDED_MODULE_RULES_CTOR
        public FMODStudio(ReadOnlyTargetRules Target) : base(Target)
    #else
        public FMODStudio(TargetInfo Target)
    #endif
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            PrivatePCHHeaderFile = "Private/FMODStudioPrivatePCH.h";

            bUseUnity = false;

            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/FMOD"));

            PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Classes"));

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "Media",
                    "Projects"
                }
                );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "MovieScene",
                    "MovieSceneTracks"
                }
                );

            if (Target.bBuildEditor == true)
            {
                PrivateDependencyModuleNames.Add("AssetRegistry");
                PrivateDependencyModuleNames.Add("UnrealEd");
                PrivateDependencyModuleNames.Add("Settings");
            }

            DynamicallyLoadedModuleNames.AddRange(
                new string[]
                {
                }
                );

            string configName = "";

            if (Target.Configuration != UnrealTargetConfiguration.Shipping)
            {
                configName = "L";
                PublicDefinitions.Add("FMODSTUDIO_LINK_LOGGING=1");
            }
            else
            {
                configName = "";
                PublicDefinitions.Add("FMODSTUDIO_LINK_RELEASE=1");
            }

            string platformName = Target.Platform.ToString();

            string linkExtension = "";
            string dllExtension = "";
            string libPrefix = "";

            // ModuleDirectory points to FMODStudio\source\FMODStudio, need to get back to binaries directory for our libs
            string BasePath = System.IO.Path.Combine(ModuleDirectory, "../../Binaries", platformName);
            // Collapse the directory path, otherwise OSX is having issues with plugin paths.
            BasePath = Utils.CleanDirectorySeparators(BasePath);

            bool bCopyToOutput = false;
            bool bAddRuntimeDependencies = true;
            bool bAddDelayLoad = false;
            bool bLinkFromBinaries = true;

            // Minimum UE version for Switch 4.15
            System.Console.WriteLine("Target Platform -- " + Target.Platform.ToString());
            if (Target.Platform == UnrealTargetPlatform.Switch)
            {
                linkExtension = ".a";
                dllExtension = ".a";
                libPrefix = "lib";
                bAddRuntimeDependencies = false;
            }
            else if (Target.Platform.ToString() == "UWP64")
            {
                linkExtension = ".lib";
                dllExtension = ".dll";
                bAddDelayLoad = true;
            }
            else if (Target.Platform == UnrealTargetPlatform.Win32)
            {
                linkExtension = "_vc.lib";
                dllExtension = ".dll";
                bAddDelayLoad = true;
            }
            else if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                linkExtension = "_vc.lib";
                dllExtension = ".dll";
                bAddDelayLoad = true;
            }
            else if (Target.Platform == UnrealTargetPlatform.Mac)
            {
                linkExtension = dllExtension = ".dylib";
                libPrefix = "lib";
                bLinkFromBinaries = false;
            }
            else if (Target.Platform == UnrealTargetPlatform.XboxOne)
            {
                linkExtension = "_vc.lib";
                dllExtension = ".dll";
                bCopyToOutput = true;
                bAddRuntimeDependencies = false;
            }
            else if (Target.Platform == UnrealTargetPlatform.PS4)
            {
                linkExtension = "_stub.a";
                dllExtension = ".prx";
                libPrefix = "lib";
                bAddDelayLoad = true;
            }
            else if (Target.Platform == UnrealTargetPlatform.Android)
            {
                bAddRuntimeDependencies = false; // Don't use this system
                bLinkFromBinaries = true;
                linkExtension = dllExtension = ".so";
                libPrefix = "lib";
            }
            else if (Target.Platform == UnrealTargetPlatform.IOS)
            {
                linkExtension = "_iphoneos.a";
                libPrefix = "lib";
                bAddRuntimeDependencies = false;
            }
            else if (Target.Platform == UnrealTargetPlatform.TVOS)
            {
                linkExtension = "_appletvos.a";
                libPrefix = "lib";
                bAddRuntimeDependencies = false;
            }
            else if (Target.Platform == UnrealTargetPlatform.Linux)
            {
                BasePath = System.IO.Path.Combine(BasePath, "x86_64");
                linkExtension = ".so";
                dllExtension = ".so";
                libPrefix = "lib";
            }
            else if (Target.Platform.ToString() == "Stadia")
            {
                linkExtension = ".so";
                dllExtension = ".so";
                libPrefix = "lib";
            }
            else
            {
                throw new System.Exception(System.String.Format("Unsupported platform {0}", Target.Platform.ToString()));
            }
            
            //System.Console.WriteLine("FMOD Current path: " + System.IO.Path.GetFullPath("."));
            //System.Console.WriteLine("FMOD Base path: " + BasePath);

            string fmodLibName = System.String.Format("{0}fmod{1}{2}", libPrefix, configName, linkExtension);
            string fmodStudioLibName = System.String.Format("{0}fmodstudio{1}{2}", libPrefix, configName, linkExtension);

            string fmodDllName = System.String.Format("{0}fmod{1}{2}", libPrefix, configName, dllExtension);
            string fmodStudioDllName = System.String.Format("{0}fmodstudio{1}{2}", libPrefix, configName, dllExtension);

            string fmodLibPath = System.IO.Path.Combine(BasePath, fmodLibName);
            string fmodStudioLibPath = System.IO.Path.Combine(BasePath, fmodStudioLibName);

            string fmodDllPath = System.IO.Path.Combine(BasePath, fmodDllName);
            string fmodStudioDllPath = System.IO.Path.Combine(BasePath, fmodStudioDllName);

            System.Collections.Generic.List<string> plugins = GetPlugins(BasePath);

            if (bLinkFromBinaries)
            {
                if (Target.IsInPlatformGroup(UnrealPlatformGroup.Android))
                {
                    string[] archs = new string[] { "armeabi-v7a", "arm64-v8a", "x86_64" };
                    foreach (string arch in archs)
                    {
                        string LibPath = System.IO.Path.Combine(BasePath, arch);
                        PublicAdditionalLibraries.Add(System.String.Format("{0}/libfmod{1}.so", LibPath, configName));
                        PublicAdditionalLibraries.Add(System.String.Format("{0}/libfmodstudio{1}.so", LibPath, configName));
                    }
                }
                else
                {
                    PublicAdditionalLibraries.Add(fmodLibPath);
                    PublicAdditionalLibraries.Add(fmodStudioLibPath);
                }
            }
            else
            {
                string LibPath = System.IO.Path.Combine(ModuleDirectory, "../../Libs/Mac/");
                PublicAdditionalLibraries.Add(System.String.Format("{0}libfmod{1}.dylib", LibPath, configName));
                PublicAdditionalLibraries.Add(System.String.Format("{0}libfmodStudio{1}.dylib", LibPath, configName));
            }

            if (bCopyToOutput)
            {
                RuntimeDependencies.Add("$(TargetOutputDir)/" + fmodDllName, fmodDllPath);
                RuntimeDependencies.Add("$(TargetOutputDir)/" + fmodStudioDllName, fmodStudioDllPath);
            }
            else if (bAddRuntimeDependencies)
            {
                RuntimeDependencies.Add(fmodDllPath);
                RuntimeDependencies.Add(fmodStudioDllPath);
                foreach (string plugin in plugins)
                {
                    string pluginPath = System.IO.Path.Combine(BasePath, plugin + dllExtension);
                    System.Console.WriteLine("Adding reference to FMOD plugin: " + pluginPath);
                    RuntimeDependencies.Add(pluginPath);
                }
            }

            if (bAddDelayLoad)
            {
                PublicDelayLoadDLLs.AddRange(
                        new string[] {
                            fmodDllName,
                            fmodStudioDllName
                            }
                        );
            }

            if (Target.Platform == UnrealTargetPlatform.Android)
            {
                string APLName = System.String.Format("FMODStudio{0}_APL.xml", configName);
                string RelAPLPath = Utils.MakePathRelativeTo(System.IO.Path.Combine(ModuleDirectory, APLName), Target.RelativeEnginePath);
                System.Console.WriteLine("Adding {0}", RelAPLPath);
                AdditionalPropertiesForReceipt.Add("AndroidPlugin", RelAPLPath);
                foreach (string PluginName in System.IO.Directory.GetFiles(BasePath))
                {
                    if (PluginName.EndsWith("_APL.xml", System.StringComparison.OrdinalIgnoreCase))
                    {
                        string RelPluginPath = Utils.MakePathRelativeTo(PluginName, Target.RelativeEnginePath);
                        System.Console.WriteLine("Adding {0}", RelPluginPath);
                        AdditionalPropertiesForReceipt.Add("AndroidPlugin", RelPluginPath);
                    }
                }
            }
        }

        private void CopyFile(string source, string dest)
        {
            //System.Console.WriteLine("Copying {0} to {1}", source, dest);
            if (System.IO.File.Exists(dest))
            {
                System.IO.File.SetAttributes(dest, System.IO.File.GetAttributes(dest) & ~System.IO.FileAttributes.ReadOnly);
            }
            try
            {
                System.IO.File.Copy(source, dest, true);
            }
            catch (System.Exception ex)
            {
                System.Console.WriteLine("Failed to copy file: {0}", ex.Message);
            }
        }

        private System.Collections.Generic.List<string> GetPlugins(string BasePath)
        {
            System.Collections.Generic.List<string> AllPlugins = new System.Collections.Generic.List<string>();
            string PluginListName = System.IO.Path.Combine(BasePath, "plugins.txt");
            if (System.IO.File.Exists(PluginListName))
            {
                try
                {
                    foreach (string FullEntry in System.IO.File.ReadAllLines(PluginListName))
                    {
                        string Entry = FullEntry.Trim();
                        if (Entry.Length > 0)
                        {
                            AllPlugins.Add(Entry);
                        }
                    }
                }
                catch (System.Exception ex)
                {
                    System.Console.WriteLine("Failed to read plugin list file: {0}", ex.Message);
                }
            }
            return AllPlugins;
        }
    }
}