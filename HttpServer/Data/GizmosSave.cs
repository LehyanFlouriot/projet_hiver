using System.Collections.Generic;

namespace INF714.Data
{
    public class GizmosSave
    {
        public List<ChestSave> Chests { get; set; } = new List<ChestSave>();
	    public List<AloneEnemySave> Enemies { get; set; } = new List<AloneEnemySave>();
	    public List<EnemySave> SubBosses { get; set; } = new List<EnemySave>();
	    public List<BossSave> Bosses { get; set; } = new List<BossSave>();
	    public List<KevinSave> Kevins { get; set; } = new List<KevinSave>();
	    public List<MercenarySave> Mercenaries { get; set; } = new List<MercenarySave>();
	    public List<GizmoSave> OtherGizmos { get; set; } = new List<GizmoSave>();
    }
}