using UnityEngine;
using UnityEngine.InputSystem;
using TMPro;

public class CalibrationReader : MonoBehaviour
{
    [SerializeField] private TMP_Text debugText;

    void Update()
    {
        var gamepad = Gamepad.current;

        if (gamepad == null)
        {
            debugText.text = "Nenhum gamepad conectado";
            return;
        }

        Vector2 leftStick = gamepad.leftStick.ReadValue();
        Vector2 rightStick = gamepad.rightStick.ReadValue();

        bool buttonSouth = gamepad.buttonSouth.isPressed;
        bool start = gamepad.startButton.isPressed;

        debugText.text =
            $"Left: {leftStick}\n" +
            $"Right: {rightStick}\n" +
            $"A: {buttonSouth}\n" +
            $"Start: {start}";
    }
}