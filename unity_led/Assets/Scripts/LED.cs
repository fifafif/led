using UnityEngine;

public class LED : MonoBehaviour
{
    [SerializeField] private SpriteRenderer spriteRenderer;

    public void SetColor(Color color)
    {
        spriteRenderer.color = color;

    }
}
