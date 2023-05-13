using System.Collections.Generic;

namespace INF714.Data
{
    public class GroupSave
    {
        public string GizmoName { get; set; }
        public Vector Tile { get; set; }
        public string GroupClassPath { get; set; }
        public List<EnemySave> Wolfs { get; set; } = new List<EnemySave>();
    
    }
}