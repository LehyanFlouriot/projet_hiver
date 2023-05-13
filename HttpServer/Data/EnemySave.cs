namespace INF714.Data
{
    public class EnemySave
    {
        public Transform Transform { get; set; }
        public string GizmoName { get; set; }
        public Vector Tile { get; set; }
        public string EnemyClassPath { get; set; }
        public float HealthPercentage { get; set; }
        public string TargetId { get; set; }
    
    }
}