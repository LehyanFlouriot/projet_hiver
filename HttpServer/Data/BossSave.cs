namespace INF714.Data
{
    public class BossSave
    {
        public Transform Transform { get; set; }
        public string GizmoName { get; set; }
        public Vector Tile { get; set; }
        public float HealthPercentage { get; set; }
        public int Phase { get; set; }    
    }
}