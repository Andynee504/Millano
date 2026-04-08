using System;
using UnityEngine;

public class DeviceConfigService : MonoBehaviour
{
    public bool IsConnected { get; private set; }
    public bool IsScanning { get; private set; }
    public string LastDeviceAddress { get; private set; }
    public string LastStatus { get; private set; } = "Idle";

    public float SensX { get; private set; } = 18f;
    public float SensY { get; private set; } = 18f;
    public float Deadzone { get; private set; } = 0.80f;
    public bool InvertX { get; private set; } = true;
    public bool InvertY { get; private set; } = false;

    public event Action<bool> ConnectionChanged;
    public event Action ConfigChanged;
    public event Action<string> StatusChanged;
    public event Action<string, string> DeviceFound;

    private BleBridge bleBridge;

    private void Awake()
    {
        bleBridge = new BleBridge();
    }

    public string ConnectionStatusLabel =>
        IsConnected ? "Dispositivo conectado" : "Dispositivo não conectado";

    public void RequestPermissions(System.Action onGranted, System.Action<string, bool> onDenied = null)
    {
        Debug.Log("[DeviceConfigService] RequestPermissions");

        BlePermissionHelper.RequestBlePermissions(onGranted: onGranted, onDenied: onDenied);
    }

    public void StartScan()
    {
        Debug.Log("[DeviceConfigService] StartScan");
        isConnecting = false;
        IsScanning = true;
        LastStatus = "Escaneando...";
        StatusChanged?.Invoke(LastStatus);
        bleBridge.StartScan();
    }

    public void StopScan()
    {
        Debug.Log("[DeviceConfigService] StopScan");
        IsScanning = false;
        bleBridge.StopScan();
        LastStatus = "Scan parado";
        StatusChanged?.Invoke(LastStatus);
    }

    public void ConnectToAddress(string address)
    {
        Debug.Log("[DeviceConfigService] ConnectToAddress -> " + address);
        LastDeviceAddress = address;
        bleBridge.ConnectToAddress(address);
    }

    public void Disconnect()
    {
        bleBridge.Disconnect();
    }

    public void SetSensX(float value)
    {
        SensX = value;
        SendCommand($"SX={value:F2}");
        ConfigChanged?.Invoke();
    }

    public void SetSensY(float value)
    {
        SensY = value;
        SendCommand($"SY={value:F2}");
        ConfigChanged?.Invoke();
    }

    public void SetDeadzone(float value)
    {
        Deadzone = value;
        SendCommand($"DZ={value:F2}");
        ConfigChanged?.Invoke();
    }

    public void SetInvertX(bool value)
    {
        InvertX = value;
        SendCommand($"IX={(value ? 1 : 0)}");
        ConfigChanged?.Invoke();
    }

    public void SetInvertY(bool value)
    {
        InvertY = value;
        SendCommand($"IY={(value ? 1 : 0)}");
        ConfigChanged?.Invoke();
    }

    public void CalibrateCenter() => SendCommand("CAL");
    public void Save() => SendCommand("SAVE");

    public void SaveAndReboot()
    {
        StartCoroutine(SaveAndRebootRoutine());
    }

    private System.Collections.IEnumerator SaveAndRebootRoutine()
    {
        SendCommand("SAVE");
        yield return new WaitForSeconds(1f); // gambiarra - criar fluxo sincrono no .java com callback
        SendCommand("REBOOT");
    }

    public void SendCommand(string command)
    {
        bleBridge.WriteCommand(command);
        LastStatus = "CMD: " + command;
        StatusChanged?.Invoke(LastStatus);
    }

    // ===== Callbacks vindos do Java via UnitySendMessage =====

    /*
    public void OnBleDeviceFound(string payload)
    {
        Debug.Log("[DeviceConfigService] OnBleDeviceFound -> " + payload);
        var parts = payload.Split('|');
        if (parts.Length < 2) return;

        DeviceFound?.Invoke(parts[0], parts[1]);
        LastStatus = "Encontrado: " + parts[0];
        StatusChanged?.Invoke(LastStatus);
    }
    */
    private bool isConnecting;

    public void OnBleDeviceFound(string payload)
    {
        Debug.Log("[DeviceConfigService] OnBleDeviceFound -> " + payload);

        if (isConnecting)
            return;

        var parts = payload.Split('|');
        if (parts.Length < 2) return;

        isConnecting = true;
        IsScanning = false;
        DeviceFound?.Invoke(parts[0], parts[1]);
        LastStatus = "Encontrado: " + parts[0];
        StatusChanged?.Invoke(LastStatus);
    }
    // TESTE

    public void OnBleStatus(string status)
    {
        Debug.Log("[DeviceConfigService] OnBleStatus -> " + status);
        LastStatus = status;
        StatusChanged?.Invoke(LastStatus);
    }

    /*
    public void OnBleConnectionChanged(string state)
    {
        Debug.Log("[DeviceConfigService] OnBleConnectionChanged -> " + state);
        IsConnected = state == "CONNECTED";
        ConnectionChanged?.Invoke(IsConnected);
        LastStatus = state;
        StatusChanged?.Invoke(LastStatus);
    }
    */
    public void OnBleConnectionChanged(string state)
    {
        Debug.Log("[DeviceConfigService] OnBleConnectionChanged -> " + state);

        IsConnected = state == "CONNECTED";

        if (state == "CONNECTED" || state == "DISCONNECTED")
        {
            isConnecting = false;
            IsScanning = false;
        }

        ConnectionChanged?.Invoke(IsConnected);
        LastStatus = state;
        StatusChanged?.Invoke(LastStatus);
    }
    // TESTE
}