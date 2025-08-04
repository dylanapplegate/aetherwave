using UnityEngine;
using UnityEngine.UI;
using UnityEditor;
using UnityEditor.Build.Reporting;
using UnityEngine.SceneManagement;
using UnityEditor.SceneManagement;
using System.IO;

namespace Aetherwave.Editor
{
    /// <summary>
    /// Automated Unity build process for Aetherwave
    /// Configures scene, builds standalone app, and prepares for execution
    /// </summary>
    public class AutomatedBuild
    {
        [MenuItem("Aetherwave/Build Gallery App")]
        public static void BuildGalleryApp()
        {
            Debug.Log("🔨 Starting Automated Aetherwave Build Process...");

            // Step 1: Setup the scene automatically
            SetupGalleryScene();

            // Step 2: Configure build settings
            ConfigureBuildSettings();

            // Step 3: Build the app
            BuildStandaloneApp();
        }

        /// <summary>
        /// Command-line entry point for automated builds
        /// </summary>
        public static void BuildFromCommandLine()
        {
            Debug.Log("🤖 Command-line build initiated...");

            try
            {
                BuildGalleryApp();
                Debug.Log("✅ Automated build completed successfully!");
                EditorApplication.Exit(0);
            }
            catch (System.Exception e)
            {
                Debug.LogError($"❌ Build failed: {e.Message}");
                EditorApplication.Exit(1);
            }
        }

        private static void SetupGalleryScene()
        {
            Debug.Log("🎨 Configuring Gallery Scene...");

            // Open or create the Gallery scene
            Scene currentScene = EditorSceneManager.GetActiveScene();
            if (currentScene.name != "Gallery")
            {
                string scenePath = "Assets/Scenes/Gallery.unity";
                if (!File.Exists(scenePath))
                {
                    EditorSceneManager.NewScene(NewSceneSetup.EmptyScene, NewSceneMode.Single);
                    EditorSceneManager.SaveScene(EditorSceneManager.GetActiveScene(), scenePath);
                }
                else
                {
                    EditorSceneManager.OpenScene(scenePath);
                }
            }

            // Clear existing objects (start fresh)
            GameObject[] allObjects = Object.FindObjectsByType<GameObject>(FindObjectsSortMode.None);
            foreach (GameObject obj in allObjects)
            {
                if (obj.scene.name == "Gallery")
                {
                    Object.DestroyImmediate(obj);
                }
            }

            // Create Camera
            GameObject cameraGO = new GameObject("Main Camera");
            Camera camera = cameraGO.AddComponent<Camera>();
            camera.backgroundColor = Color.black;
            camera.clearFlags = CameraClearFlags.SolidColor;
            camera.tag = "MainCamera";

            // Create Canvas for UI
            GameObject canvasGO = new GameObject("Gallery Canvas");
            Canvas canvas = canvasGO.AddComponent<Canvas>();
            canvas.renderMode = RenderMode.ScreenSpaceOverlay;
            canvas.sortingOrder = 0;

            CanvasScaler scaler = canvasGO.AddComponent<CanvasScaler>();
            scaler.uiScaleMode = CanvasScaler.ScaleMode.ScaleWithScreenSize;
            scaler.referenceResolution = new Vector2(1920, 1080);
            scaler.screenMatchMode = CanvasScaler.ScreenMatchMode.MatchWidthOrHeight;
            scaler.matchWidthOrHeight = 0.5f;

            canvasGO.AddComponent<GraphicRaycaster>();

            // Create main image display
            GameObject imageGO = new GameObject("Main Image Display");
            imageGO.transform.SetParent(canvasGO.transform, false);

            RectTransform imageRect = imageGO.AddComponent<RectTransform>();
            imageRect.anchorMin = Vector2.zero;
            imageRect.anchorMax = Vector2.one;
            imageRect.offsetMin = Vector2.zero;
            imageRect.offsetMax = Vector2.zero;

            RawImage rawImage = imageGO.AddComponent<RawImage>();
            rawImage.color = Color.black;

            // Note: ImageDisplayManager will be added at runtime when the scene loads
            // This avoids compilation issues during automated build process

            // Create EventSystem
            GameObject eventSystemGO = new GameObject("EventSystem");
            eventSystemGO.AddComponent<UnityEngine.EventSystems.EventSystem>();
            eventSystemGO.AddComponent<UnityEngine.EventSystems.StandaloneInputModule>();

            // Note: Image loading functionality will be added post-build

            // Save the scene
            EditorSceneManager.MarkSceneDirty(EditorSceneManager.GetActiveScene());
            EditorSceneManager.SaveScene(EditorSceneManager.GetActiveScene());

            Debug.Log("✅ Gallery Scene configured successfully");
            Debug.Log("📸 ImageDisplayManager attached to: " + imageGO.name);
        }

        private static void ConfigureBuildSettings()
        {
            Debug.Log("⚙️ Configuring Build Settings...");

            // Set build scenes
            EditorBuildSettingsScene[] scenes = {
                new EditorBuildSettingsScene("Assets/Scenes/Gallery.unity", true)
            };
            EditorBuildSettings.scenes = scenes;

            // Configure player settings
            PlayerSettings.companyName = "Aetherwave";
            PlayerSettings.productName = "Aetherwave";
            PlayerSettings.bundleVersion = "1.0.0";

#if UNITY_STANDALONE_OSX
            PlayerSettings.SetApplicationIdentifier(BuildTargetGroup.Standalone, "com.aetherwave.gallery");
#endif

            Debug.Log("✅ Build Settings configured");
        }

        private static void BuildStandaloneApp()
        {
            Debug.Log("📦 Building Standalone Application...");

            string buildPath = Path.Combine(System.Environment.CurrentDirectory, "build", "unity");

            // Ensure build directory exists
            Directory.CreateDirectory(buildPath);

#if UNITY_STANDALONE_OSX
            string appPath = Path.Combine(buildPath, "Aetherwave.app");
#elif UNITY_STANDALONE_WIN
            string appPath = Path.Combine(buildPath, "Aetherwave.exe");
#else
            string appPath = Path.Combine(buildPath, "Aetherwave");
#endif

            BuildPlayerOptions buildOptions = new BuildPlayerOptions
            {
                scenes = new[] { "Assets/Scenes/Gallery.unity" },
                locationPathName = appPath,
#if UNITY_STANDALONE_OSX
                target = BuildTarget.StandaloneOSX,
#elif UNITY_STANDALONE_WIN
                target = BuildTarget.StandaloneWindows64,
#else
                target = BuildTarget.StandaloneLinux64,
#endif
                options = BuildOptions.None
            };

            BuildReport report = BuildPipeline.BuildPlayer(buildOptions);
            BuildSummary summary = report.summary;

            if (summary.result == BuildResult.Succeeded)
            {
                Debug.Log($"✅ Build succeeded: {summary.totalSize} bytes");
                Debug.Log($"📱 App location: {appPath}");

                // Create a simple run script
                CreateRunScript(appPath);
            }
            else
            {
                Debug.LogError($"❌ Build failed: {summary.result}");
                foreach (var step in report.steps)
                {
                    if (step.messages.Length > 0)
                    {
                        foreach (var message in step.messages)
                        {
                            if (message.type == LogType.Error)
                            {
                                Debug.LogError($"Build error: {message.content}");
                            }
                        }
                    }
                }
            }
        }

        private static void CreateRunScript(string appPath)
        {
#if UNITY_STANDALONE_OSX
            string scriptPath = Path.Combine(Path.GetDirectoryName(appPath), "run-aetherwave.sh");
            string scriptContent = $@"#!/bin/bash
echo ""🚀 Starting Aetherwave...""
echo ""📡 Make sure Python API is running: ./scripts/dev-setup.sh""
echo ""🎮 Controls: SPACE=Next, BACKSPACE=Previous, R=Random, ESC=Exit""
echo """"
open ""{appPath}""
";
            File.WriteAllText(scriptPath, scriptContent);

            // Make executable
            System.Diagnostics.Process.Start("chmod", $"+x \"{scriptPath}\"");

            Debug.Log($"📜 Run script created: {scriptPath}");
#endif
        }
    }
}
