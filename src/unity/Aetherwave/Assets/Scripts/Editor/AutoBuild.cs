using UnityEngine;
using UnityEditor;
using UnityEditor.Build.Reporting;
using System.IO;

public class AutoBuild
{
    [MenuItem("Build/Build OSX App")]
    public static void BuildOSXApp()
    {
        BuildPlayerOptions buildPlayerOptions = new BuildPlayerOptions();
        buildPlayerOptions.scenes = new[] { "Assets/Scenes/Gallery.unity" };

        // Create a simple path in the project root
        string projectPath = Application.dataPath.Replace("/Assets", "");
        string buildPath = Path.Combine(projectPath, "../../build/bin/Aetherwave.app");
        buildPath = Path.GetFullPath(buildPath);  // Normalize the path

        buildPlayerOptions.locationPathName = buildPath;
        buildPlayerOptions.target = BuildTarget.StandaloneOSX;
        buildPlayerOptions.options = BuildOptions.None;

        Debug.Log("AutoBuild: Starting build...");
        Debug.Log("AutoBuild: Project path: " + projectPath);
        Debug.Log("AutoBuild: Build target: " + buildPath);

        // Ensure the output directory exists
        string outputDir = Path.GetDirectoryName(buildPath);
        if (!Directory.Exists(outputDir))
        {
            Directory.CreateDirectory(outputDir);
            Debug.Log("AutoBuild: Created output directory: " + outputDir);
        }

        BuildReport report = BuildPipeline.BuildPlayer(buildPlayerOptions);
        BuildSummary summary = report.summary;

        if (summary.result == BuildResult.Succeeded)
        {
            Debug.Log("AutoBuild: Build succeeded: " + summary.outputPath + " (" + summary.totalSize + " bytes)");
        }
        else
        {
            Debug.LogError("AutoBuild: Build failed with result: " + summary.result);
            Debug.LogError("AutoBuild: Total errors: " + summary.totalErrors);
            Debug.LogError("AutoBuild: Total warnings: " + summary.totalWarnings);
        }
    }

    // Command-line build method
    public static void BuildFromCommandLine()
    {
        Debug.Log("AutoBuild: Starting command-line build...");
        BuildOSXApp();
        Debug.Log("AutoBuild: Build complete, exiting Unity...");
    }
}
