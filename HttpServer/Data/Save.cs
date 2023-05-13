using System.Collections.Generic;

namespace INF714.Data
{
    public class Save
    {
        public GameSave GameSave { get; set; }

        public DirectorSave DirectorSave { get; set; }

        public Dictionary<string, PlayerSave> PlayerSaves { get; } = new Dictionary<string, PlayerSave>();
    }
}