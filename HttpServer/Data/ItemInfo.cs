namespace INF714.Data
{
    public class ItemInfo
    {
        public int Id { get; set; }
        public string RowName { get; set; }
        public string ItemType { get; set; }
        public int Level { get; set; }
        public int BuyValue { get; set; }
        public int SellValue { get; set; }
        public int Stack { get; set; }
        public ItemStats ItemStats { get; set; }
        public string Ability1 { get; set; }
        public string Ability1ClassPath { get; set; }
        public string Ability2 { get; set; }
        public string Ability2ClassPath { get; set; }
    }
}