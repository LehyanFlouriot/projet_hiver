namespace INF714.Data
{
    public class UserScoreboard
    {
        public int DamageDealt { get; set; }
        public int DamageReceived { get; set; }
        public int Knockouts { get; set; }
        public int Resurrections { get; set; }
        public int Deaths { get; set; }
        public int PotionsDrank { get; set; }
        public int EnemiesKilled { get; set; }
        public int BossesKilled { get; set; }
        public int GamesWon { get; set; }
        public int ItemsBought { get; set; }
        public int ItemsSold { get; set; }
        public int ItemsPickedUp { get; set; }
        public int ItemsUpgraded { get; set; }
        public int TilesExplored { get; set; }
        public int ContractsCompleted { get; set; }

        public static UserScoreboard operator+(UserScoreboard a, UserScoreboard b)
        {
            var scoreboard = new UserScoreboard();

            scoreboard.DamageDealt = a.DamageDealt + b.DamageDealt;
            scoreboard.DamageReceived = a.DamageReceived + b.DamageReceived;
            scoreboard.Knockouts = a.Knockouts + b.Knockouts;
            scoreboard.Resurrections = a.Resurrections + b.Resurrections;
            scoreboard.Deaths = a.Deaths + b.Deaths;
            scoreboard.PotionsDrank = a.PotionsDrank + b.PotionsDrank;
            scoreboard.EnemiesKilled = a.EnemiesKilled + b.EnemiesKilled;
            scoreboard.GamesWon = a.GamesWon + b.GamesWon;
            scoreboard.ItemsBought = a.ItemsBought + b.ItemsBought;
            scoreboard.ItemsSold = a.ItemsSold + b.ItemsSold;
            scoreboard.ItemsPickedUp = a.ItemsPickedUp + b.ItemsPickedUp;
            scoreboard.ItemsUpgraded = a.ItemsUpgraded + b.ItemsUpgraded;
            scoreboard.TilesExplored = a.TilesExplored + b.TilesExplored;
            scoreboard.ContractsCompleted = a.ContractsCompleted + b.ContractsCompleted;
            
            return scoreboard;
        }
    }
}
