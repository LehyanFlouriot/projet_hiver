using System.Collections.Generic;

namespace INF714.Data
{
    public class GameSave
    {
        public string GameId { get; set; }

        public string GameName { get; set; }

        public string GameCreatorId { get; set; }

        public string Date { get; set; }

        public List<ItemInfo> SharedInventory { get; set; } = new List<ItemInfo>();

        public int Money { get; set; }

        public Transform InventoryTransform { get; set; }

        public string TargetId { get; set; }

        public int Seed { get; set; }

        public int CurrentLevel { get; set; }

        public float DamageTaken { get; set; }

        public int NbVisitedTiles { get; set; }

        public List<string> ContractClassPaths { get; set; } = new List<string>();

        public List<string> ContractStatuses { get; set; } = new List<string>();

        public List<int> ContractCurrentValues { get; set; } = new List<int>();

        public int HealAllPlayersCost { get; set; }
    }
}
