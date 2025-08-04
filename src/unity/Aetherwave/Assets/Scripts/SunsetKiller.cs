using UnityEngine;

namespace Aetherwave
{
    /// <summary>
    /// Emergency script to kill the sunset background
    /// This script aggressively removes all skybox materials and forces solid color
    /// </summary>
    public class SunsetKiller : MonoBehaviour
    {
        void Awake()
        {
            Debug.Log("🔥 SunsetKiller: KILLING THE SUNSET!");
            KillSunset();
        }

        void Start()
        {
            Debug.Log("🔥 SunsetKiller: Start() - KILLING THE SUNSET AGAIN!");
            KillSunset();
        }

        void Update()
        {
            // Kill sunset every frame for the first 5 seconds
            if (Time.time < 5f)
            {
                KillSunset();
            }
        }

        void KillSunset()
        {
            // Nuclear option: destroy all skybox references
            RenderSettings.skybox = null;
            RenderSettings.defaultReflectionMode = UnityEngine.Rendering.DefaultReflectionMode.Custom;
            RenderSettings.customReflection = null;
            DynamicGI.UpdateEnvironment();

            // Force ALL cameras to solid color
            Camera[] allCameras = FindObjectsOfType<Camera>();
            foreach (Camera cam in allCameras)
            {
                cam.clearFlags = CameraClearFlags.SolidColor;
                cam.backgroundColor = Color.magenta; // Use bright magenta to confirm this script is working
                Debug.Log($"🔥 SunsetKiller: Set camera {cam.name} to MAGENTA background");
            }

            Debug.Log($"🔥 SunsetKiller: Skybox = {RenderSettings.skybox}, Found {allCameras.Length} cameras");
        }
    }
}
