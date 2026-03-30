using System.Collections.Generic;
using UnityEngine;
using TMPro;

public class ScreenRouter : MonoBehaviour
{
    [SerializeField] private UIScreen initialScreen;
    [SerializeField] private List<UIScreen> screens = new();

    private UIScreen currentScreen;

    private void Start()
    {
        foreach (var screen in screens)
        {
            if (screen != null)
                screen.gameObject.SetActive(false);
        }

        if (initialScreen != null)
            Open(initialScreen);
    }

    public void Open(UIScreen target)
    {
        if (target == null || target == currentScreen) return;

        if (currentScreen != null)
            currentScreen.Hide();

        currentScreen = target;
        currentScreen.Show();
    }
}