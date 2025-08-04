using UnityEngine;
using UnityEngine.SceneManagement;

namespace Aetherwave
{
    /// <summary>
    /// Debug script to understand what's happening at startup
    /// </summary>
    public static class StartupDebugger
    {
        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
        static void DebugBeforeSceneLoad()
        {
            Debug.Log($"🐛 StartupDebugger: BEFORE SCENE LOAD");
            Debug.Log($"🐛 Current scene: {SceneManager.GetActiveScene().name}");
            Debug.Log($"🐛 RenderSettings.skybox: {RenderSettings.skybox}");

            Camera[] cameras = Object.FindObjectsOfType<Camera>();
            Debug.Log($"🐛 Found {cameras.Length} cameras:");
            foreach (Camera cam in cameras)
            {
                Debug.Log($"🐛   - Camera: {cam.name}, ClearFlags: {cam.clearFlags}, BG: {cam.backgroundColor}");
            }
        }

        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.AfterSceneLoad)]
        static void DebugAfterSceneLoad()
        {
            Debug.Log($"🐛 StartupDebugger: AFTER SCENE LOAD");
            Debug.Log($"🐛 Current scene: {SceneManager.GetActiveScene().name}");
            Debug.Log($"🐛 RenderSettings.skybox: {RenderSettings.skybox}");

            Camera[] cameras = Object.FindObjectsOfType<Camera>();
            Debug.Log($"🐛 Found {cameras.Length} cameras:");
            foreach (Camera cam in cameras)
            {
                Debug.Log($"🐛   - Camera: {cam.name}, ClearFlags: {cam.clearFlags}, BG: {cam.backgroundColor}");
            }

            // List all GameObjects with MonoBehaviour scripts
            MonoBehaviour[] scripts = Object.FindObjectsOfType<MonoBehaviour>();
            Debug.Log($"🐛 Found {scripts.Length} MonoBehaviour scripts:");
            foreach (MonoBehaviour script in scripts)
            {
                Debug.Log($"🐛   - Script: {script.GetType().Name} on {script.gameObject.name}");
            }
        }
    }
}
