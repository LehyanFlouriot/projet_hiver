namespace INF714.Data
{
    public class PlayerSave
    {
        public string PlayerId { get; set; }
        public ItemInfo ArmorKit { get; set; }
        public ItemInfo WeaponKit { get; set; }
        public ItemInfo Belt { get; set; }
        public ItemInfo Gloves { get; set; }
        public ItemInfo Ring { get; set; }
        public ItemInfo Earrings { get; set; }
        public ItemInfo Necklace { get; set; }
        public Transform Transform { get; set; }
        public float HealthPercentage { get; set; }
	    public int NbPotionCharges { get; set; }
	    public bool bIsDead { get; set; }
	    public int CurrentLevel { get; set; }
    }
}