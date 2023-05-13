using System.Collections.Generic;

namespace INF714.Data
{
    public class DirectorSave
    {
        public int IntensityState { get; set; }
        public int CurrentNumberOfRoomVisited { get; set; }
        public float CurrentDifficulty { get; set; }
        public List<Vector> VisitedTiles { get; set; }  = new List<Vector>();
        public List<Vector> LoadedTiles { get; set; }  = new List<Vector>();
        public GizmosSave GizmosSave  { get; set; }
    }
}