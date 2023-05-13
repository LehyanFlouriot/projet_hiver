using Amazon.DynamoDBv2;
using Amazon.DynamoDBv2.Model;
using INF714.Services;
using System;
using System.Threading.Tasks;
using System.Collections.Generic;

namespace INF714.Data.Providers
{
    public class DynamoDbUserProvider : Interfaces.IUserProvider
    {
        readonly AmazonDynamoDBClient _client;
        const string _tableName = "efhoris_users";

        public DynamoDbUserProvider(DynamoDbService dynamoDbService)
        {
            _client = dynamoDbService.Client;
        }

        #region Serialization Boilerplate

        private Dictionary<string, AttributeValue> ConvertVectorToDynamoDbItem(Vector userVector)
        {
            var vector = new Dictionary<string, AttributeValue>();
            vector.Add("x", new AttributeValue() { S = userVector.X.ToString() });
            vector.Add("y", new AttributeValue() { S = userVector.Y.ToString() });
            vector.Add("z", new AttributeValue() { S = userVector.Z.ToString() });

            return vector;
        }

        private Vector ConvertVectorFromDynamoDbItem(Dictionary<string, AttributeValue> vector)
        {
            var userVector = new Vector();
            AttributeValue value;
            if(vector.TryGetValue("x", out value)) userVector.X = float.Parse(value.S);
            if(vector.TryGetValue("y", out value)) userVector.Y = float.Parse(value.S);
            if(vector.TryGetValue("z", out value)) userVector.Z = float.Parse(value.S);

            return userVector;
        }

        private Dictionary<string, AttributeValue> ConvertQuatToDynamoDbItem(Quat userQuat)
        {
            var quat = new Dictionary<string, AttributeValue>();
            quat.Add("x", new AttributeValue() { S = userQuat.X.ToString() });
            quat.Add("y", new AttributeValue() { S = userQuat.Y.ToString() });
            quat.Add("z", new AttributeValue() { S = userQuat.Z.ToString() });
            quat.Add("w", new AttributeValue() { S = userQuat.W.ToString() });

            return quat;
        }

        private Quat ConvertQuatFromDynamoDbItem(Dictionary<string, AttributeValue> quat)
        {
            var userQuat = new Quat();
            AttributeValue value;
            if(quat.TryGetValue("x", out value)) userQuat.X = float.Parse(value.S);
            if(quat.TryGetValue("y", out value)) userQuat.Y = float.Parse(value.S);
            if(quat.TryGetValue("z", out value)) userQuat.Z = float.Parse(value.S);
            if(quat.TryGetValue("w", out value)) userQuat.W = float.Parse(value.S);

            return userQuat;
        }

        private Dictionary<string, AttributeValue> ConvertTransformToDynamoDbItem(Transform userTrasnform)
        {
            var transform = new Dictionary<string, AttributeValue>();
            transform.Add("rotation", new AttributeValue() { M = ConvertQuatToDynamoDbItem(userTrasnform.Rotation) });
            transform.Add("translation", new AttributeValue() { M = ConvertVectorToDynamoDbItem(userTrasnform.Translation) });
            transform.Add("scale3D", new AttributeValue() { M = ConvertVectorToDynamoDbItem(userTrasnform.Scale3D) });

            return transform;
        }

        private Transform ConvertTransformFromDynamoDbItem(Dictionary<string, AttributeValue> transform)
        {
            var userTrasnform = new Transform();
            AttributeValue value;
            if(transform.TryGetValue("rotation", out value)) userTrasnform.Rotation = ConvertQuatFromDynamoDbItem(value.M);
            if(transform.TryGetValue("translation", out value)) userTrasnform.Translation = ConvertVectorFromDynamoDbItem(value.M);
            if(transform.TryGetValue("scale3D", out value)) userTrasnform.Scale3D = ConvertVectorFromDynamoDbItem(value.M);

            return userTrasnform;
        }

        private Dictionary<string, AttributeValue> ConvertChestSaveToDynamoDbItem(ChestSave userChestSave)
        {
            var chestSave = new Dictionary<string, AttributeValue>();
            chestSave.Add("bOpened", new AttributeValue() { BOOL = userChestSave.bOpened });
            chestSave.Add("gizmoName", new AttributeValue() { S = userChestSave.GizmoName });
            chestSave.Add("tile", new AttributeValue() { M = ConvertVectorToDynamoDbItem(userChestSave.Tile) });

            return chestSave;
        }

        private ChestSave ConvertChestSaveFromDynamoDbItem(Dictionary<string, AttributeValue> chestSave)
        {
            var userChestSave = new ChestSave();
            AttributeValue value;
            if(chestSave.TryGetValue("bOpened", out value)) userChestSave.bOpened = value.BOOL;
            if(chestSave.TryGetValue("gizmoName", out value)) userChestSave.GizmoName = value.S;
            if(chestSave.TryGetValue("tile", out value)) userChestSave.Tile = ConvertVectorFromDynamoDbItem(value.M);

            return userChestSave;
        }

        private Dictionary<string, AttributeValue> ConvertEnemySaveToDynamoDbItem(EnemySave userEnemySave)
        {
            var enemySave = new Dictionary<string, AttributeValue>();
            enemySave.Add("transform", new AttributeValue() { M = ConvertTransformToDynamoDbItem(userEnemySave.Transform) });
            enemySave.Add("gizmoName", new AttributeValue() { S = userEnemySave.GizmoName });
            enemySave.Add("tile", new AttributeValue() { M = ConvertVectorToDynamoDbItem(userEnemySave.Tile) });
            enemySave.Add("enemyClassPath", new AttributeValue() { S = userEnemySave.EnemyClassPath });
            enemySave.Add("healthPercentage", new AttributeValue() { S = userEnemySave.HealthPercentage.ToString() });
            enemySave.Add("targetId", new AttributeValue() { S = userEnemySave.TargetId });

            return enemySave;
        }

        private EnemySave ConvertEnemySaveFromDynamoDbItem(Dictionary<string, AttributeValue> enemySave)
        {
            var userEnemySave = new EnemySave();
            AttributeValue value;
            if(enemySave.TryGetValue("transform", out value)) userEnemySave.Transform = ConvertTransformFromDynamoDbItem(value.M);
            if(enemySave.TryGetValue("gizmoName", out value)) userEnemySave.GizmoName = value.S;
            if(enemySave.TryGetValue("tile", out value)) userEnemySave.Tile = ConvertVectorFromDynamoDbItem(value.M);
            if(enemySave.TryGetValue("enemyClassPath", out value)) userEnemySave.EnemyClassPath = value.S;
            if(enemySave.TryGetValue("healthPercentage", out value)) userEnemySave.HealthPercentage = float.Parse(value.S);
            if(enemySave.TryGetValue("targetId", out value)) userEnemySave.TargetId = value.S;

            return userEnemySave;
        }

        private Dictionary<string, AttributeValue> ConvertGroupSaveToDynamoDbItem(GroupSave userGroupSave)
        {
            var groupSave = new Dictionary<string, AttributeValue>();
            groupSave.Add("gizmoName", new AttributeValue() { S = userGroupSave.GizmoName });
            groupSave.Add("tile", new AttributeValue() { M = ConvertVectorToDynamoDbItem(userGroupSave.Tile) });
            groupSave.Add("groupClassPath", new AttributeValue() { S = userGroupSave.GroupClassPath });
            var wolfs = new List<AttributeValue>();
            foreach(var wolf in userGroupSave.Wolfs) {
                wolfs.Add(new AttributeValue() { M = ConvertEnemySaveToDynamoDbItem(wolf)});
            }
            if(wolfs.Count != 0) groupSave.Add("wolfs", new AttributeValue() { L = wolfs });

            return groupSave;
        }

        private GroupSave ConvertGroupSaveFromDynamoDbItem(Dictionary<string, AttributeValue> groupSave)
        {
            var userGroupSave = new GroupSave();
            AttributeValue value;
            if(groupSave.TryGetValue("gizmoName", out value)) userGroupSave.GizmoName = value.S;
            if(groupSave.TryGetValue("tile", out value)) userGroupSave.Tile = ConvertVectorFromDynamoDbItem(value.M);
            if(groupSave.TryGetValue("groupClassPath", out value)) userGroupSave.GroupClassPath = value.S;
            if(groupSave.TryGetValue("wolfs", out value))
            {
                foreach(var wolf in value.L)
                {
                    userGroupSave.Wolfs.Add(ConvertEnemySaveFromDynamoDbItem(wolf.M));
                }
            }

            return userGroupSave;
        }

        private Dictionary<string, AttributeValue> ConvertAloneEnemySaveToDynamoDbItem(AloneEnemySave userAloneEnemy)
        {
            var aloneEnemySave = new Dictionary<string, AttributeValue>();
            if(userAloneEnemy.bIsGroup)
                aloneEnemySave.Add("group", new AttributeValue() { M = ConvertGroupSaveToDynamoDbItem(userAloneEnemy.Group) });
            else
                aloneEnemySave.Add("alone", new AttributeValue() { M = ConvertEnemySaveToDynamoDbItem(userAloneEnemy.Enemy) });

            return aloneEnemySave;
        }

        private AloneEnemySave ConvertAloneEnemySaveFromDynamoDbItem(Dictionary<string, AttributeValue> aloneEnemySave)
        {
            var userAloneEnemy = new AloneEnemySave();
            AttributeValue value;
            if(aloneEnemySave.TryGetValue("group", out value)) {
                userAloneEnemy.bIsGroup = true;
                userAloneEnemy.Group = ConvertGroupSaveFromDynamoDbItem(value.M);
            }
            if(aloneEnemySave.TryGetValue("alone", out value)) userAloneEnemy.Enemy = ConvertEnemySaveFromDynamoDbItem(value.M);

            return userAloneEnemy;
        }

        private Dictionary<string, AttributeValue> ConvertBossSaveToDynamoDbItem(BossSave userBoss)
        {
            var bossSave = new Dictionary<string, AttributeValue>();
            bossSave.Add("transform", new AttributeValue() { M = ConvertTransformToDynamoDbItem(userBoss.Transform) });
            bossSave.Add("gizmoName", new AttributeValue() { S = userBoss.GizmoName });
            bossSave.Add("tile", new AttributeValue() { M = ConvertVectorToDynamoDbItem(userBoss.Tile) });
            bossSave.Add("healthPercentage", new AttributeValue() { S = userBoss.HealthPercentage.ToString() });
            bossSave.Add("phase", new AttributeValue() { N = userBoss.Phase.ToString() });

            return bossSave;
        }

        private BossSave ConvertBossSaveFromDynamoDbItem(Dictionary<string, AttributeValue> bossSave)
        {
            var userBoss = new BossSave();
            AttributeValue value;
            if(bossSave.TryGetValue("transform", out value)) userBoss.Transform = ConvertTransformFromDynamoDbItem(value.M);
            if(bossSave.TryGetValue("gizmoName", out value)) userBoss.GizmoName = value.S;
            if(bossSave.TryGetValue("tile", out value)) userBoss.Tile = ConvertVectorFromDynamoDbItem(value.M);
            if(bossSave.TryGetValue("healthPercentage", out value)) userBoss.HealthPercentage = float.Parse(value.S);
            if(bossSave.TryGetValue("phase", out value)) userBoss.Phase = int.Parse(value.N);

            return userBoss;
        }

        private Dictionary<string, AttributeValue> ConvertKevinSaveToDynamoDbItem(KevinSave userKevin)
        {
            var kevinSave = new Dictionary<string, AttributeValue>();
            kevinSave.Add("transform", new AttributeValue() { M = ConvertTransformToDynamoDbItem(userKevin.Transform) });
            kevinSave.Add("gizmoName", new AttributeValue() { S = userKevin.GizmoName });
            kevinSave.Add("tile", new AttributeValue() { M = ConvertVectorToDynamoDbItem(userKevin.Tile) });
            kevinSave.Add("targetId", new AttributeValue() { S = userKevin.TargetId });


            return kevinSave;
        }

        private KevinSave ConvertKevinSaveFromDynamoDbItem(Dictionary<string, AttributeValue> kevinSave)
        {
            var userKevin = new KevinSave();
            AttributeValue value;
            if(kevinSave.TryGetValue("transform", out value)) userKevin.Transform = ConvertTransformFromDynamoDbItem(value.M);
            if(kevinSave.TryGetValue("gizmoName", out value)) userKevin.GizmoName = value.S;
            if(kevinSave.TryGetValue("tile", out value)) userKevin.Tile = ConvertVectorFromDynamoDbItem(value.M);
            if(kevinSave.TryGetValue("targetId", out value)) userKevin.TargetId = value.S;

            return userKevin;
        }

        private Dictionary<string, AttributeValue> ConvertMercenarySaveToDynamoDbItem(MercenarySave userMercenarySave)
        {
            var mercenarySave = new Dictionary<string, AttributeValue>();
            mercenarySave.Add("transform", new AttributeValue() { M = ConvertTransformToDynamoDbItem(userMercenarySave.Transform) });
            mercenarySave.Add("gizmoName", new AttributeValue() { S = userMercenarySave.GizmoName });
            mercenarySave.Add("tile", new AttributeValue() { M = ConvertVectorToDynamoDbItem(userMercenarySave.Tile) });
            mercenarySave.Add("healthPercentage", new AttributeValue() { S = userMercenarySave.HealthPercentage.ToString() });
            mercenarySave.Add("nbPotionCharges", new AttributeValue() { S = userMercenarySave.NbPotionCharges.ToString() });
            mercenarySave.Add("masterId", new AttributeValue() { S = userMercenarySave.MasterId });

            return mercenarySave;
        }

        private MercenarySave ConvertMercenarySaveFromDynamoDbItem(Dictionary<string, AttributeValue> mercenarySave)
        {
            var userMercenarySave = new MercenarySave();
            AttributeValue value;
            if(mercenarySave.TryGetValue("transform", out value)) userMercenarySave.Transform = ConvertTransformFromDynamoDbItem(value.M);
            if(mercenarySave.TryGetValue("gizmoName", out value)) userMercenarySave.GizmoName = value.S;
            if(mercenarySave.TryGetValue("tile", out value)) userMercenarySave.Tile = ConvertVectorFromDynamoDbItem(value.M);
            if(mercenarySave.TryGetValue("healthPercentage", out value)) userMercenarySave.HealthPercentage = float.Parse(value.S);
            if(mercenarySave.TryGetValue("nbPotionCharges", out value)) userMercenarySave.NbPotionCharges = float.Parse(value.S);
            if(mercenarySave.TryGetValue("masterId", out value)) userMercenarySave.MasterId = value.S;

            return userMercenarySave;
        }

        private Dictionary<string, AttributeValue> ConvertGizmoSaveToDynamoDbItem(GizmoSave userGizmo)
        {
            var gizmoSave = new Dictionary<string, AttributeValue>();
            gizmoSave.Add("type", new AttributeValue() { S = userGizmo.Type });
            gizmoSave.Add("gizmoName", new AttributeValue() { S = userGizmo.GizmoName });
            gizmoSave.Add("tile", new AttributeValue() { M = ConvertVectorToDynamoDbItem(userGizmo.Tile) });

            return gizmoSave;
        }

        private GizmoSave ConvertGizmoSaveFromDynamoDbItem(Dictionary<string, AttributeValue> gizmoSave)
        {
            var userGizmo = new GizmoSave();
            AttributeValue value;
            if(gizmoSave.TryGetValue("type", out value)) userGizmo.Type = value.S;
            if(gizmoSave.TryGetValue("gizmoName", out value)) userGizmo.GizmoName = value.S;
            if(gizmoSave.TryGetValue("tile", out value)) userGizmo.Tile = ConvertVectorFromDynamoDbItem(value.M);

            return userGizmo;
        }

        private Dictionary<string, AttributeValue> ConvertGizmosSaveToDynamoDbItem(GizmosSave userGizmos)
        {
            var gizmosSave = new Dictionary<string, AttributeValue>();
            var chests = new List<AttributeValue>();
            foreach(var chest in userGizmos.Chests) {
                chests.Add(new AttributeValue() { M = ConvertChestSaveToDynamoDbItem(chest)});
            }
            if(chests.Count != 0) gizmosSave.Add("chests", new AttributeValue() { L = chests });
            var enemies = new List<AttributeValue>();
            foreach(var enemy in userGizmos.Enemies) {
                enemies.Add(new AttributeValue() { M = ConvertAloneEnemySaveToDynamoDbItem(enemy)});
            }
            if(enemies.Count != 0) gizmosSave.Add("enemies", new AttributeValue() { L = enemies });
            var subBosses = new List<AttributeValue>();
            foreach(var subBoss in userGizmos.SubBosses) {
                subBosses.Add(new AttributeValue() { M = ConvertEnemySaveToDynamoDbItem(subBoss)});
            }
            if(subBosses.Count != 0) gizmosSave.Add("subBosses", new AttributeValue() { L = subBosses });
            var bosses = new List<AttributeValue>();
            foreach(var boss in userGizmos.Bosses) {
                bosses.Add(new AttributeValue() { M = ConvertBossSaveToDynamoDbItem(boss)});
            }
            if(bosses.Count != 0) gizmosSave.Add("bosses", new AttributeValue() { L = bosses });
            var kevins = new List<AttributeValue>();
            foreach(var kevin in userGizmos.Kevins) {
                kevins.Add(new AttributeValue() { M = ConvertKevinSaveToDynamoDbItem(kevin)});
            }
            if(kevins.Count != 0) gizmosSave.Add("kevins", new AttributeValue() { L = kevins });
            var mercenaries = new List<AttributeValue>();
            foreach(var mercenary in userGizmos.Mercenaries) {
                mercenaries.Add(new AttributeValue() { M = ConvertMercenarySaveToDynamoDbItem(mercenary)});
            }
            if(mercenaries.Count != 0) gizmosSave.Add("mercenaries", new AttributeValue() { L = mercenaries });
            var otherGizmos = new List<AttributeValue>();
            foreach(var giz in userGizmos.OtherGizmos) {
                otherGizmos.Add(new AttributeValue() { M = ConvertGizmoSaveToDynamoDbItem(giz)});
            }
            if(otherGizmos.Count != 0) gizmosSave.Add("otherGizmos", new AttributeValue() { L = otherGizmos });

            return gizmosSave;
        }

        private GizmosSave ConvertGizmosSaveFromDynamoDbItem(Dictionary<string, AttributeValue> gizmosSave)
        {
            var userGizmos = new GizmosSave();
            AttributeValue value;
            if(gizmosSave.TryGetValue("chests", out value))
            {
                foreach(var chest in value.L)
                {
                    userGizmos.Chests.Add(ConvertChestSaveFromDynamoDbItem(chest.M));
                }
            }
            if(gizmosSave.TryGetValue("enemies", out value))
            {
                foreach(var enemy in value.L)
                {
                    userGizmos.Enemies.Add(ConvertAloneEnemySaveFromDynamoDbItem(enemy.M));
                }
            }
            if(gizmosSave.TryGetValue("subBosses", out value))
            {
                foreach(var subBoss in value.L)
                {
                    userGizmos.SubBosses.Add(ConvertEnemySaveFromDynamoDbItem(subBoss.M));
                }
            }
            if(gizmosSave.TryGetValue("bosses", out value))
            {
                foreach(var boss in value.L)
                {
                    userGizmos.Bosses.Add(ConvertBossSaveFromDynamoDbItem(boss.M));
                }
            }
            if(gizmosSave.TryGetValue("kevins", out value))
            {
                foreach(var kevin in value.L)
                {
                    userGizmos.Kevins.Add(ConvertKevinSaveFromDynamoDbItem(kevin.M));
                }
            }
            if(gizmosSave.TryGetValue("mercenaries", out value))
            {
                foreach(var mercenary in value.L)
                {
                    userGizmos.Mercenaries.Add(ConvertMercenarySaveFromDynamoDbItem(mercenary.M));
                }
            }
            if(gizmosSave.TryGetValue("otherGizmos", out value))
            {
                foreach(var giz in value.L)
                {
                    userGizmos.OtherGizmos.Add(ConvertGizmoSaveFromDynamoDbItem(giz.M));
                }
            }

            return userGizmos;
        }

        private Dictionary<string, AttributeValue> ConvertDirectorSaveToDynamoDbItem(DirectorSave userDirectorSave)
        {
            var directorSave = new Dictionary<string, AttributeValue>();
            directorSave.Add("intensityState", new AttributeValue() { N = userDirectorSave.IntensityState.ToString() });
            directorSave.Add("currentNumberOfRoomVisited", new AttributeValue() { N = userDirectorSave.CurrentNumberOfRoomVisited.ToString() });
            directorSave.Add("currentDifficulty", new AttributeValue() { S = userDirectorSave.CurrentDifficulty.ToString() });
            var visited = new List<AttributeValue>();
            foreach(var tile in userDirectorSave.VisitedTiles) {
                visited.Add(new AttributeValue() { M = ConvertVectorToDynamoDbItem(tile)});
            }
            if(visited.Count != 0) directorSave.Add("visitedTiles", new AttributeValue() { L = visited });
            var loaded = new List<AttributeValue>();
            foreach(var tile in userDirectorSave.LoadedTiles) {
                loaded.Add(new AttributeValue() { M = ConvertVectorToDynamoDbItem(tile)});
            }
            if(loaded.Count != 0) directorSave.Add("loadedTiles", new AttributeValue() { L = loaded });
            directorSave.Add("gizmosSave", new AttributeValue() { M = ConvertGizmosSaveToDynamoDbItem(userDirectorSave.GizmosSave) });

            return directorSave;
        }

        private DirectorSave ConvertDirectorSaveFromDynamoDbItem(Dictionary<string, AttributeValue> directorSave)
        {
            var userDirectorSave = new DirectorSave();
            AttributeValue value;
            if(directorSave.TryGetValue("intensityState", out value)) userDirectorSave.IntensityState = int.Parse(value.N);
            if(directorSave.TryGetValue("currentNumberOfRoomVisited", out value)) userDirectorSave.CurrentNumberOfRoomVisited = int.Parse(value.N);
            if(directorSave.TryGetValue("currentDifficulty", out value)) userDirectorSave.CurrentDifficulty = float.Parse(value.S);
            if(directorSave.TryGetValue("visitedTiles", out value))
            {
                foreach(var tile in value.L)
                {
                    userDirectorSave.VisitedTiles.Add(ConvertVectorFromDynamoDbItem(tile.M));
                }
            }
            if(directorSave.TryGetValue("loadedTiles", out value))
            {
                foreach(var tile in value.L)
                {
                    userDirectorSave.LoadedTiles.Add(ConvertVectorFromDynamoDbItem(tile.M));
                }
            }
            if(directorSave.TryGetValue("gizmosSave", out value)) userDirectorSave.GizmosSave = ConvertGizmosSaveFromDynamoDbItem(value.M);

            return userDirectorSave;
        }

        private Dictionary<string, AttributeValue> ConvertItemStatsToDynamoDbItem(ItemStats userItemStats)
        {
            var itemStats = new Dictionary<string, AttributeValue>();
            itemStats.Add("physicalPower", new AttributeValue() { N = userItemStats.PhysicalPower.ToString() });
            itemStats.Add("physicalDefense", new AttributeValue() { N = userItemStats.PhysicalDefense.ToString() });
            itemStats.Add("blockingPhysicalDefense", new AttributeValue() { N = userItemStats.BlockingPhysicalDefense.ToString() });
            itemStats.Add("magicalPower", new AttributeValue() { N = userItemStats.MagicalPower.ToString() });
            itemStats.Add("magicalDefense", new AttributeValue() { N = userItemStats.MagicalDefense.ToString() });
            itemStats.Add("blockingMagicalDefense", new AttributeValue() { N = userItemStats.BlockingMagicalDefense.ToString() });
            itemStats.Add("maxHealth", new AttributeValue() { N = userItemStats.MaxHealth.ToString() });
            itemStats.Add("maxPotionCharge", new AttributeValue() { N = userItemStats.MaxPotionCharge.ToString() });
            itemStats.Add("movementSpeed", new AttributeValue() { S = userItemStats.MovementSpeed.ToString() });

            return itemStats;
        }

        private ItemStats ConvertItemStatsFromDynamoDbItem(Dictionary<string, AttributeValue> itemStats)
        {
            var userItemStats = new ItemStats();
            AttributeValue value;
            if(itemStats.TryGetValue("physicalPower", out value)) userItemStats.PhysicalPower = int.Parse(value.N);
            if(itemStats.TryGetValue("physicalDefense", out value)) userItemStats.PhysicalDefense = int.Parse(value.N);
            if(itemStats.TryGetValue("blockingPhysicalDefense", out value)) userItemStats.BlockingPhysicalDefense = int.Parse(value.N);
            if(itemStats.TryGetValue("magicalPower", out value)) userItemStats.MagicalPower = int.Parse(value.N);
            if(itemStats.TryGetValue("magicalDefense", out value)) userItemStats.MagicalDefense = int.Parse(value.N);
            if(itemStats.TryGetValue("blockingMagicalDefense", out value)) userItemStats.BlockingMagicalDefense = int.Parse(value.N);
            if(itemStats.TryGetValue("maxHealth", out value)) userItemStats.MaxHealth = int.Parse(value.N);
            if(itemStats.TryGetValue("maxPotionCharge", out value)) userItemStats.MaxPotionCharge = int.Parse(value.N);
            if(itemStats.TryGetValue("movementSpeed", out value)) userItemStats.MovementSpeed = float.Parse(value.S);

            return userItemStats;
        }

        private Dictionary<string, AttributeValue> ConvertItemInfoToDynamoDbItem(ItemInfo userItemInfo)
        {
            var itemInfo = new Dictionary<string, AttributeValue>();
            itemInfo.Add("id", new AttributeValue() { N = userItemInfo.Id.ToString() });
            if(!string.IsNullOrEmpty(userItemInfo.RowName)) itemInfo.Add("rowName", new AttributeValue() { S = userItemInfo.RowName });
            if(!string.IsNullOrEmpty(userItemInfo.ItemType)) itemInfo.Add("itemType", new AttributeValue() { S = userItemInfo.ItemType });
            itemInfo.Add("level", new AttributeValue() { N = userItemInfo.Level.ToString() });
            itemInfo.Add("buyValue", new AttributeValue() { N = userItemInfo.BuyValue.ToString() });
            itemInfo.Add("sellValue", new AttributeValue() { N = userItemInfo.SellValue.ToString() });
            itemInfo.Add("stack", new AttributeValue() { N = userItemInfo.Stack.ToString() });
            itemInfo.Add("itemStats", new AttributeValue() { M = ConvertItemStatsToDynamoDbItem(userItemInfo.ItemStats) });
            if(!string.IsNullOrEmpty(userItemInfo.Ability1)) itemInfo.Add("ability1", new AttributeValue() { S = userItemInfo.Ability1 });
            if(!string.IsNullOrEmpty(userItemInfo.Ability1ClassPath)) itemInfo.Add("ability1ClassPath", new AttributeValue() { S = userItemInfo.Ability1ClassPath });
            if(!string.IsNullOrEmpty(userItemInfo.Ability2)) itemInfo.Add("ability2", new AttributeValue() { S = userItemInfo.Ability2 });
            if(!string.IsNullOrEmpty(userItemInfo.Ability2ClassPath)) itemInfo.Add("ability2ClassPath", new AttributeValue() { S = userItemInfo.Ability2ClassPath });

            return itemInfo;
        }

        private ItemInfo ConvertItemInfoFromDynamoDbItem(Dictionary<string, AttributeValue> itemInfo)
        {
            var userItemInfo = new ItemInfo();
            AttributeValue value;
            if(itemInfo.TryGetValue("id", out value)) userItemInfo.Id = int.Parse(value.N);
            if(itemInfo.TryGetValue("rowName", out value)) userItemInfo.RowName = value.S;
            if(itemInfo.TryGetValue("itemType", out value)) userItemInfo.ItemType = value.S;
            if(itemInfo.TryGetValue("level", out value)) userItemInfo.Level = int.Parse(value.N);
            if(itemInfo.TryGetValue("buyValue", out value)) userItemInfo.BuyValue = int.Parse(value.N);
            if(itemInfo.TryGetValue("sellValue", out value)) userItemInfo.SellValue = int.Parse(value.N);
            if(itemInfo.TryGetValue("stack", out value)) userItemInfo.Stack = int.Parse(value.N);
            if(itemInfo.TryGetValue("itemStats", out value)) userItemInfo.ItemStats = ConvertItemStatsFromDynamoDbItem(value.M);
            if(itemInfo.TryGetValue("ability1", out value)) userItemInfo.Ability1 = value.S;
            if(itemInfo.TryGetValue("ability1ClassPath", out value)) userItemInfo.Ability1ClassPath = value.S;
            if(itemInfo.TryGetValue("ability2", out value)) userItemInfo.Ability2 = value.S;
            if(itemInfo.TryGetValue("ability2ClassPath", out value)) userItemInfo.Ability2ClassPath = value.S;

            return userItemInfo;
        }

        private Dictionary<string, AttributeValue> ConvertPlayerSaveToDynamoDbItem(PlayerSave userPlayerSave)
        {
            var playersave = new Dictionary<string, AttributeValue>();
            playersave.Add("playerId", new AttributeValue() { S = userPlayerSave.PlayerId });
            playersave.Add("armorKit", new AttributeValue() { M = ConvertItemInfoToDynamoDbItem(userPlayerSave.ArmorKit) });
            playersave.Add("weaponKit", new AttributeValue() { M = ConvertItemInfoToDynamoDbItem(userPlayerSave.WeaponKit) });
            playersave.Add("belt", new AttributeValue() { M = ConvertItemInfoToDynamoDbItem(userPlayerSave.Belt) });
            playersave.Add("gloves", new AttributeValue() { M = ConvertItemInfoToDynamoDbItem(userPlayerSave.Gloves) });
            playersave.Add("ring", new AttributeValue() { M = ConvertItemInfoToDynamoDbItem(userPlayerSave.Ring) });
            playersave.Add("earrings", new AttributeValue() { M = ConvertItemInfoToDynamoDbItem(userPlayerSave.Earrings) });
            playersave.Add("necklace", new AttributeValue() { M = ConvertItemInfoToDynamoDbItem(userPlayerSave.Necklace) });
            playersave.Add("transform", new AttributeValue() { M = ConvertTransformToDynamoDbItem(userPlayerSave.Transform) });
            playersave.Add("healthPercentage", new AttributeValue() { S = userPlayerSave.HealthPercentage.ToString() });
            playersave.Add("nbPotionCharges", new AttributeValue() { N = userPlayerSave.NbPotionCharges.ToString() });
            playersave.Add("isDead", new AttributeValue() { BOOL = userPlayerSave.bIsDead });
            playersave.Add("currentLevel", new AttributeValue() { N = userPlayerSave.CurrentLevel.ToString() });

            return playersave;
        }

        private PlayerSave ConvertPlayerSaveFromDynamoDbItem(Dictionary<string, AttributeValue> playerSave)
        {
            var userPlayerSave = new PlayerSave();
            AttributeValue value;
            if(playerSave.TryGetValue("playerId", out value)) userPlayerSave.PlayerId = value.S;
            if(playerSave.TryGetValue("armorKit", out value)) userPlayerSave.ArmorKit = ConvertItemInfoFromDynamoDbItem(value.M);
            if(playerSave.TryGetValue("weaponKit", out value)) userPlayerSave.WeaponKit = ConvertItemInfoFromDynamoDbItem(value.M);
            if(playerSave.TryGetValue("belt", out value)) userPlayerSave.Belt = ConvertItemInfoFromDynamoDbItem(value.M);
            if(playerSave.TryGetValue("gloves", out value)) userPlayerSave.Gloves = ConvertItemInfoFromDynamoDbItem(value.M);
            if(playerSave.TryGetValue("ring", out value)) userPlayerSave.Ring = ConvertItemInfoFromDynamoDbItem(value.M);
            if(playerSave.TryGetValue("earrings", out value)) userPlayerSave.Earrings = ConvertItemInfoFromDynamoDbItem(value.M);
            if(playerSave.TryGetValue("necklace", out value)) userPlayerSave.Necklace = ConvertItemInfoFromDynamoDbItem(value.M);
            if(playerSave.TryGetValue("transform", out value)) userPlayerSave.Transform = ConvertTransformFromDynamoDbItem(value.M);
            if(playerSave.TryGetValue("healthPercentage", out value)) userPlayerSave.HealthPercentage = float.Parse(value.S);
            if(playerSave.TryGetValue("nbPotionCharges", out value)) userPlayerSave.NbPotionCharges = int.Parse(value.N);
            if(playerSave.TryGetValue("isDead", out value)) userPlayerSave.bIsDead = value.BOOL;
            if(playerSave.TryGetValue("currentLevel", out value)) userPlayerSave.CurrentLevel = int.Parse(value.N);

            return userPlayerSave;
        }

        private Dictionary<string, AttributeValue> ConvertGameSaveToDynamoDbItem(GameSave userGameSave)
        {
            var gameSave = new Dictionary<string, AttributeValue>();

            gameSave.Add("gameId", new AttributeValue() { S = userGameSave.GameId });
            if(!string.IsNullOrEmpty(userGameSave.GameName)) gameSave.Add("gameName", new AttributeValue() { S = userGameSave.GameName });
            if(!string.IsNullOrEmpty(userGameSave.GameCreatorId)) gameSave.Add("gameCreatordId", new AttributeValue() { S = userGameSave.GameCreatorId });
            if(!string.IsNullOrEmpty(userGameSave.Date)) gameSave.Add("date", new AttributeValue() { S = userGameSave.Date });
            var inventory = new List<AttributeValue>();
            foreach(var userItem in userGameSave.SharedInventory) {
                inventory.Add(new AttributeValue() { M = ConvertItemInfoToDynamoDbItem(userItem)});
            }
            if(inventory.Count != 0) gameSave.Add("sharedInventory", new AttributeValue() { L = inventory });
            if(!string.IsNullOrEmpty(userGameSave.TargetId)) gameSave.Add("targetId", new AttributeValue() { S = userGameSave.TargetId });
            gameSave.Add("money", new AttributeValue() { N = userGameSave.Money.ToString() });
            gameSave.Add("inventoryTransform", new AttributeValue() { M = ConvertTransformToDynamoDbItem(userGameSave.InventoryTransform) });
            gameSave.Add("seed", new AttributeValue() { N = userGameSave.Seed.ToString() });
            gameSave.Add("currentLevel", new AttributeValue() { N = userGameSave.CurrentLevel.ToString() });
            gameSave.Add("damageTaken", new AttributeValue() { S = userGameSave.DamageTaken.ToString() });
            gameSave.Add("nbVisitedTiles", new AttributeValue() { N = userGameSave.NbVisitedTiles.ToString() });
            var classPaths = new List<AttributeValue>();
            foreach(var classPath in userGameSave.ContractClassPaths) {
                classPaths.Add(new AttributeValue() { S = classPath });
            }
            if(classPaths.Count != 0) gameSave.Add("contractClassPaths", new AttributeValue() { L = classPaths });
            var contractStatus = new List<AttributeValue>();
            foreach(var status in userGameSave.ContractStatuses) {
                contractStatus.Add(new AttributeValue() { S = status });
            }
            if(contractStatus.Count != 0) gameSave.Add("contractStatuses", new AttributeValue() { L = contractStatus });
            var currentValues = new List<AttributeValue>();
            foreach(var currentValue in userGameSave.ContractCurrentValues) {
                currentValues.Add(new AttributeValue() { N = currentValue.ToString() });
            }
            if(currentValues.Count != 0) gameSave.Add("contractCurrentValues", new AttributeValue() { L = currentValues });
            gameSave.Add("healAllPlayersCost", new AttributeValue() { N = userGameSave.HealAllPlayersCost.ToString() });

            return gameSave;
        }

        private GameSave ConvertGameSaveFromDynamoDbItem(Dictionary<string, AttributeValue> save)
        {
            var userGameSave = new GameSave();
            AttributeValue value;
            if(save.TryGetValue("gameId", out value)) userGameSave.GameId = value.S;
            if(save.TryGetValue("gameName", out value)) userGameSave.GameName = value.S;
            if(save.TryGetValue("gameCreatordId", out value)) userGameSave.GameCreatorId = value.S;
            if(save.TryGetValue("date", out value)) userGameSave.Date = value.S;
            if(save.TryGetValue("sharedInventory", out value))
            {
                foreach(var item in value.L)
                {
                    userGameSave.SharedInventory.Add(ConvertItemInfoFromDynamoDbItem(item.M));
                }
            }
            if(save.TryGetValue("targetId", out value)) userGameSave.TargetId = value.S;
            if(save.TryGetValue("money", out value)) userGameSave.Money = int.Parse(value.N);
            if(save.TryGetValue("inventoryTransform", out value)) userGameSave.InventoryTransform = ConvertTransformFromDynamoDbItem(value.M);
            if(save.TryGetValue("seed", out value)) userGameSave.Seed = int.Parse(value.N);
            if(save.TryGetValue("currentLevel", out value)) userGameSave.CurrentLevel = int.Parse(value.N);
            if(save.TryGetValue("damageTaken", out value)) userGameSave.DamageTaken = float.Parse(value.S);
            if(save.TryGetValue("nbVisitedTiles", out value)) userGameSave.NbVisitedTiles = int.Parse(value.N);
            if(save.TryGetValue("contractClassPaths", out value))
            {
                foreach(var classPath in value.L)
                {
                    userGameSave.ContractClassPaths.Add(classPath.S);
                }
            }
            if(save.TryGetValue("contractStatuses", out value))
            {
                foreach(var contractStatus in value.L)
                {
                    userGameSave.ContractStatuses.Add(contractStatus.S);
                }
            }
            if(save.TryGetValue("contractCurrentValues", out value))
            {
                foreach(var currentValue in value.L)
                {
                    userGameSave.ContractCurrentValues.Add(int.Parse(currentValue.N));
                }
            }
            if(save.TryGetValue("healAllPlayersCost", out value)) userGameSave.HealAllPlayersCost = int.Parse(value.N);

            return userGameSave;
        }

        private Dictionary<string, AttributeValue> ConvertSaveToDynamoDbItem(Save userSave)
        {
            var save = new Dictionary<string, AttributeValue>();
            save.Add("gameSave", new AttributeValue() { M = ConvertGameSaveToDynamoDbItem(userSave.GameSave) });
            if(userSave.DirectorSave != null)save.Add("directorSave", new AttributeValue() { M = ConvertDirectorSaveToDynamoDbItem(userSave.DirectorSave) });
            var playerSaves = new Dictionary<string, AttributeValue>();
            foreach(var playerSave in userSave.PlayerSaves) {
                playerSaves.Add(playerSave.Key, new AttributeValue() { M = ConvertPlayerSaveToDynamoDbItem(playerSave.Value) });
            }
            if(playerSaves.Count != 0) save.Add("playerSaves", new AttributeValue() { M = playerSaves });
            return save;
        }

        private Save ConvertSaveFromDynamoDbItem(Dictionary<string, AttributeValue> save)
        {
            var userSave = new Save();
            AttributeValue value;
            if(save.TryGetValue("gameSave", out value)) userSave.GameSave = ConvertGameSaveFromDynamoDbItem(value.M);
            if(save.TryGetValue("directorSave", out value)) userSave.DirectorSave = ConvertDirectorSaveFromDynamoDbItem(value.M);
            if(save.TryGetValue("playerSaves", out value))
            {
                foreach(var playerSavePair in value.M)
                {
                    userSave.PlayerSaves.Add(playerSavePair.Key, ConvertPlayerSaveFromDynamoDbItem(playerSavePair.Value.M));
                }
            }

            return userSave;
        }

        private Dictionary<string, AttributeValue> ConvertUserScoreboardToDynamoDbItem(UserScoreboard userScoreboard)
        {
            var scoreboard = new Dictionary<string, AttributeValue>();

            scoreboard.Add("DamageDealt", new AttributeValue() { N = userScoreboard.DamageDealt.ToString() });
            scoreboard.Add("DamageReceived", new AttributeValue() { N = userScoreboard.DamageReceived.ToString() });
            scoreboard.Add("Knockouts", new AttributeValue() { N = userScoreboard.Knockouts.ToString() });
            scoreboard.Add("Resurrections", new AttributeValue() { N = userScoreboard.Resurrections.ToString() });
            scoreboard.Add("Deaths", new AttributeValue() { N = userScoreboard.Deaths.ToString() });
            scoreboard.Add("PotionsDrank", new AttributeValue() { N = userScoreboard.PotionsDrank.ToString() });
            scoreboard.Add("EnemiesKilled", new AttributeValue() { N = userScoreboard.EnemiesKilled.ToString() });
            scoreboard.Add("BossesKilled", new AttributeValue() { N = userScoreboard.BossesKilled.ToString() });
            scoreboard.Add("GamesWon", new AttributeValue() { N = userScoreboard.GamesWon.ToString() });
            scoreboard.Add("ItemsBought", new AttributeValue() { N = userScoreboard.ItemsBought.ToString() });
            scoreboard.Add("ItemsSold", new AttributeValue() { N = userScoreboard.ItemsSold.ToString() });
            scoreboard.Add("ItemsPickedUp", new AttributeValue() { N = userScoreboard.ItemsPickedUp.ToString() });
            scoreboard.Add("ItemsUpgraded", new AttributeValue() { N = userScoreboard.ItemsUpgraded.ToString() });
            scoreboard.Add("TilesExplored", new AttributeValue() { N = userScoreboard.TilesExplored.ToString() });
            scoreboard.Add("ContractsCompleted", new AttributeValue() { N = userScoreboard.ContractsCompleted.ToString() });

            return scoreboard;
        }

        private UserScoreboard ConvertUserScoreboardFromDynamoDbItem(Dictionary<string, AttributeValue> scoreboard)
        {
            var userScoreboard = new UserScoreboard();
            AttributeValue value;

            if (scoreboard.TryGetValue("DamageDealt", out value)) userScoreboard.DamageDealt = int.Parse(value.N);
            if (scoreboard.TryGetValue("DamageReceived", out value)) userScoreboard.DamageReceived = int.Parse(value.N);
            if (scoreboard.TryGetValue("Knockouts", out value)) userScoreboard.Knockouts = int.Parse(value.N);
            if (scoreboard.TryGetValue("Resurrections", out value)) userScoreboard.Resurrections = int.Parse(value.N);
            if (scoreboard.TryGetValue("Deaths", out value)) userScoreboard.Deaths = int.Parse(value.N);
            if (scoreboard.TryGetValue("PotionsDrank", out value)) userScoreboard.PotionsDrank = int.Parse(value.N);
            if (scoreboard.TryGetValue("EnemiesKilled", out value)) userScoreboard.EnemiesKilled = int.Parse(value.N);
            if (scoreboard.TryGetValue("BossesKilled", out value)) userScoreboard.BossesKilled = int.Parse(value.N);
            if (scoreboard.TryGetValue("GamesWon", out value)) userScoreboard.GamesWon = int.Parse(value.N);
            if (scoreboard.TryGetValue("ItemsBought", out value)) userScoreboard.ItemsBought = int.Parse(value.N);
            if (scoreboard.TryGetValue("ItemsSold", out value)) userScoreboard.ItemsSold = int.Parse(value.N);
            if (scoreboard.TryGetValue("ItemsPickedUp", out value)) userScoreboard.ItemsPickedUp = int.Parse(value.N);
            if (scoreboard.TryGetValue("ItemsUpgraded", out value)) userScoreboard.ItemsUpgraded = int.Parse(value.N);
            if (scoreboard.TryGetValue("TilesExplored", out value)) userScoreboard.TilesExplored = int.Parse(value.N);
            if (scoreboard.TryGetValue("ContractsCompleted", out value)) userScoreboard.ContractsCompleted = int.Parse(value.N);
            
            return userScoreboard;
        }

        private Dictionary<string, AttributeValue> ConvertUserToDynamoDbItem(User user)
        {
            var item = new Dictionary<string, AttributeValue>();
            item.Add("id", new AttributeValue() { S = user.Id });
            item.Add("guestToken", new AttributeValue() { S = user.GuestToken });
            if(!string.IsNullOrEmpty(user.Name)) item.Add("name", new AttributeValue() { S = user.Name });

            var userSavesSave = new Dictionary<string, AttributeValue>();
            foreach(var userSavePair in user.Saves)
            {
                userSavesSave.Add(userSavePair.Key, new AttributeValue() { M = ConvertSaveToDynamoDbItem(userSavePair.Value) });
            }
            if(userSavesSave.Count != 0) item.Add("saves", new AttributeValue() { M = userSavesSave });

            item.Add("scoreboard", new AttributeValue() { M = ConvertUserScoreboardToDynamoDbItem(user.Scoreboard) });

            return item;
        }

        private User ConvertUserFromDynamoDbItem(Dictionary<string, AttributeValue> item)
        {
            var user = new User();
            AttributeValue value;
            if(item.TryGetValue("id", out value)) user.Id = value.S;
            if(item.TryGetValue("guestToken", out value)) user.GuestToken = value.S;
            if(item.TryGetValue("name", out value)) user.Name = value.S;
            if(item.TryGetValue("saves", out value))
            {
                foreach(var savePair in value.M)
                {
                    user.Saves.Add(savePair.Key, ConvertSaveFromDynamoDbItem(savePair.Value.M));
                }
            }

            if (item.TryGetValue("scoreboard", out value)) user.Scoreboard = ConvertUserScoreboardFromDynamoDbItem(value.M);

            return user;
        }
        
        #endregion

        public async Task Create(User user)
        {
            var request = new PutItemRequest();
            request.TableName = _tableName;
            request.ConditionExpression = "attribute_not_exists(id)";
            request.Item = ConvertUserToDynamoDbItem(user);
            await _client.PutItemAsync(request);
        }

        public async Task<User> Get(string id)
        {
            var key = new Dictionary<string, AttributeValue>();
            key.Add("id", new AttributeValue() { S = id });

            var request = new GetItemRequest();
            request.TableName = _tableName;
            request.Key = key;
            var response = await _client.GetItemAsync(request);

            if(!response.IsItemSet)
            {
                return null;
            }

            return ConvertUserFromDynamoDbItem(response.Item);
        }

        public async Task Save(User user)
        {
            var request = new PutItemRequest();
            request.TableName = _tableName;
            request.Item = ConvertUserToDynamoDbItem(user);
            await _client.PutItemAsync(request);
        }
    }
}
