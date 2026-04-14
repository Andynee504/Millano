using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class ProfileScreen : UIScreen
{
    [Header("Services")]
    [SerializeField] private DeviceConfigService deviceConfigService;

    [Header("Status")]
    [SerializeField] private TMP_Text statusText;

    [Header("Profile labels")]
    [SerializeField] private TMP_Text profile1Label;
    [SerializeField] private TMP_Text profile2Label;
    [SerializeField] private TMP_Text profile3Label;

    [Header("Save buttons")]
    [SerializeField] private Button saveProfile1Button;
    [SerializeField] private Button saveProfile2Button;
    [SerializeField] private Button saveProfile3Button;

    [Header("Load buttons")]
    [SerializeField] private Button loadProfile1Button;
    [SerializeField] private Button loadProfile2Button;
    [SerializeField] private Button loadProfile3Button;

    private void Awake()
    {
        AddButtonListener(saveProfile1Button, () => SaveSlot(1));
        AddButtonListener(saveProfile2Button, () => SaveSlot(2));
        AddButtonListener(saveProfile3Button, () => SaveSlot(3));

        AddButtonListener(loadProfile1Button, () => LoadSlot(1));
        AddButtonListener(loadProfile2Button, () => LoadSlot(2));
        AddButtonListener(loadProfile3Button, () => LoadSlot(3));
    }

    private void OnEnable()
    {
        if (deviceConfigService != null)
            deviceConfigService.StatusChanged += HandleStatusChanged;

        RefreshUI();
    }

    private void OnDisable()
    {
        if (deviceConfigService != null)
            deviceConfigService.StatusChanged -= HandleStatusChanged;
    }

    protected override void OnEnter()
    {
        RefreshUI();
    }

    private void HandleStatusChanged(string status)
    {
        if (statusText != null)
            statusText.text = status;
    }

    private void SaveSlot(int slot)
    {
        if (deviceConfigService == null)
            return;

        deviceConfigService.SaveProfile(slot);
        RefreshUI();
    }

    private void LoadSlot(int slot)
    {
        if (deviceConfigService == null)
            return;

        deviceConfigService.LoadProfile(slot);
        RefreshUI();
    }

    private void RefreshUI()
    {
        RefreshLabel(profile1Label, 1);
        RefreshLabel(profile2Label, 2);
        RefreshLabel(profile3Label, 3);

        bool canLoad1 = deviceConfigService != null && deviceConfigService.HasSavedProfile(1);
        bool canLoad2 = deviceConfigService != null && deviceConfigService.HasSavedProfile(2);
        bool canLoad3 = deviceConfigService != null && deviceConfigService.HasSavedProfile(3);

        if (loadProfile1Button != null)
            loadProfile1Button.interactable = canLoad1;

        if (loadProfile2Button != null)
            loadProfile2Button.interactable = canLoad2;

        if (loadProfile3Button != null)
            loadProfile3Button.interactable = canLoad3;
    }

    private void RefreshLabel(TMP_Text label, int slot)
    {
        if (label == null)
            return;

        bool hasProfile = deviceConfigService != null && deviceConfigService.HasSavedProfile(slot);
        label.text = hasProfile ? $"Perfil {slot} • salvo" : $"Perfil {slot} • vazio";
        // ― U+2015 Horizontal Bar
        // → U+2192 Right Arrow
        // • U+2022 Bullet
    }

    private static void AddButtonListener(Button button, UnityEngine.Events.UnityAction action)
    {
        if (button == null)
            return;

        button.onClick.AddListener(action);
    }
}
