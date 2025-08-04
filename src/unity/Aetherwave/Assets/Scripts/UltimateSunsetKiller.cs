using UnityEngine;

namespace Aetherwave
{
    /// <summary>
    /// Ultimate sunset killer - executes before any scene loads
    /// This script forces itself to run regardless of scene setup
    /// </summary>
    public static class UltimateSunsetKiller
    {
        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
        static void KillSunsetBeforeSceneLoad()
        {
            Debug.Log("🔥🔥 UltimateSunsetKiller: BEFORE SCENE LOAD - KILLING SUNSET!");
            ForceKillSunset();
        }

        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.AfterSceneLoad)]
        static void KillSunsetAfterSceneLoad()
        {
            Debug.Log("🔥🔥 UltimateSunsetKiller: AFTER SCENE LOAD - KILLING SUNSET AGAIN!");
            ForceKillSunset();

            // Create a persistent GameObject to continue killing sunset
            GameObject sunsetKiller = new GameObject("SUNSET_KILLER_PERSISTENT");
            sunsetKiller.AddComponent<PersistentSunsetKiller>();
            Object.DontDestroyOnLoad(sunsetKiller);
        }

        static void ForceKillSunset()
        {
            // Nuclear option: destroy all skybox references
            RenderSettings.skybox = null;
            RenderSettings.defaultReflectionMode = UnityEngine.Rendering.DefaultReflectionMode.Custom;
            RenderSettings.customReflection = null;
            DynamicGI.UpdateEnvironment();

            // Force ALL cameras to bright green so we can see if this works
            Camera[] allCameras = Object.FindObjectsOfType<Camera>();
            foreach (Camera cam in allCameras)
            {
                cam.clearFlags = CameraClearFlags.SolidColor;
                cam.backgroundColor = Color.green; // Bright green to confirm this is working
                Debug.Log($"🔥🔥 UltimateSunsetKiller: Set camera {cam.name} to GREEN background");
            }

            Debug.Log($"🔥🔥 UltimateSunsetKiller: Skybox = {RenderSettings.skybox}, Found {allCameras.Length} cameras");
        }
    }

    public class PersistentSunsetKiller : MonoBehaviour
    {
        private float killTimer = 0f;

        void Update()
        {
            // Kill sunset every 0.1 seconds for the first 10 seconds
            killTimer += Time.deltaTime;
            if (killTimer >= 0.1f && Time.time < 10f)
            {
                killTimer = 0f;

                RenderSettings.skybox = null;
                DynamicGI.UpdateEnvironment();

                Camera[] allCameras = FindObjectsOfType<Camera>();
                foreach (Camera cam in allCameras)
                {
                    cam.clearFlags = CameraClearFlags.SolidColor;
                    cam.backgroundColor = Color.green; // Keep it green to confirm working
                }
            }
        }
    }
}
