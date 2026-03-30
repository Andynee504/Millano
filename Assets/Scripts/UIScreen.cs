using UnityEngine;
public class UIScreen : MonoBehaviour
{
    public virtual void Show()
    {
        gameObject.SetActive(true);
        OnEnter();
    }

    public virtual void Hide()
    {
        OnExit();
        gameObject.SetActive(false);
    }

    protected virtual void OnEnter() { }
    protected virtual void OnExit() { }
}