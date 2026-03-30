using UnityEngine;

public class ScreenNavBtn : MonoBehaviour
{
    [SerializeField] private ScreenRouter router;
    [SerializeField] private UIScreen targetScreen;

    public void Navigate()
    {
        if (router == null || targetScreen == null) return;
        router.Open(targetScreen);
    }
}