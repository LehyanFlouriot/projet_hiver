using System.ComponentModel.DataAnnotations;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Text.Json;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using INF714.Data;
using INF714.Data.Providers.Interfaces;

namespace INF714.Controllers
{

    [Route("api/users/{userId}/[controller]")]
    [ApiController]
    public class SavesController : ControllerBase
    {
        private IUserProvider _userProvider;

        public SavesController(IUserProvider userProvider)
        {
            _userProvider = userProvider;
        }

        [Authorize("CanAccessSelfInfo")]
        [HttpPost("{saveId}/store")]
        public async Task<ActionResult> StoreGame(string userId, string saveId, [FromForm] [Required] string gameSave) {
            var user = await _userProvider.Get(userId);
            if(user == null) return NotFound();

            JsonSerializerOptions policy = new JsonSerializerOptions();
            policy.PropertyNameCaseInsensitive = true;

            GameSave jsonGameSave = JsonSerializer.Deserialize<GameSave>(gameSave, policy);
            if(!user.Saves.ContainsKey(saveId)) {
                var save = new Save();
                save.GameSave = jsonGameSave;
                user.Saves.Add(saveId, save);
            } else {
                user.Saves[saveId].GameSave = jsonGameSave;
            }
            await _userProvider.Save(user);

            return Ok(user);
        }

        [Authorize("CanAccessSelfInfo")]
        [HttpPost("{saveId}/storeAI")]
        public async Task<ActionResult> StoreDirector(string userId, string saveId, [FromForm] [Required] string directorSave) {
            var user = await _userProvider.Get(userId);
            if(user == null) return NotFound();
            if(!user.Saves.ContainsKey(saveId)) return NotFound(saveId);

            JsonSerializerOptions policy = new JsonSerializerOptions();
            policy.PropertyNameCaseInsensitive = true;

            user.Saves[saveId].DirectorSave = JsonSerializer.Deserialize<DirectorSave>(directorSave, policy);
            await _userProvider.Save(user);

            return Ok(user);
        }

        [Authorize("CanAccessSelfInfo")]
        [HttpPost("{saveId}/{playerId}/store")]
        public async Task<ActionResult> StorePlayer(string userId, string saveId, string playerId, [FromForm] [Required] string playerSave) {
            var user = await _userProvider.Get(userId);
            if(user == null) return NotFound();
            if(!user.Saves.ContainsKey(saveId)) return NotFound(saveId);

            JsonSerializerOptions policy = new JsonSerializerOptions();
            policy.PropertyNameCaseInsensitive = true;

            PlayerSave jsonPlayerSave = JsonSerializer.Deserialize<PlayerSave>(playerSave, policy);
            if(!user.Saves[saveId].PlayerSaves.ContainsKey(playerId)) {
                user.Saves[saveId].PlayerSaves.Add(playerId, jsonPlayerSave);
            } else {
                user.Saves[saveId].PlayerSaves[playerId] = jsonPlayerSave;
            }
            await _userProvider.Save(user);

            return Ok(playerId);
        }

        [Authorize("CanAccessSelfInfo")]
        [HttpGet("{saveId}/loadAI")]
        public async Task<ActionResult> LoadDirectorSave(string userId, string saveId) {
            var user = await _userProvider.Get(userId);
            if(user == null) return NotFound();

            JsonSerializerOptions policy = new JsonSerializerOptions();
            policy.PropertyNameCaseInsensitive = true;

            string directorSave = JsonSerializer.Serialize<DirectorSave>(user.Saves[saveId].DirectorSave, policy);

            return Ok(directorSave);
        }

        [Authorize("CanAccessSelfInfo")]
        [HttpGet("{saveId}/loadPlayers")]
        public async Task<List<string>> LoadPlayerSaves(string userId, string saveId) {
            var user = await _userProvider.Get(userId);
            List<string> playerSaveList = new List<string>();
            if(user == null) return playerSaveList;

            JsonSerializerOptions policy = new JsonSerializerOptions();
            policy.PropertyNameCaseInsensitive = true;

            foreach(var playerSavePair in user.Saves[saveId].PlayerSaves) {
                string playerSave = JsonSerializer.Serialize<PlayerSave>(playerSavePair.Value, policy);
                playerSaveList.Add(playerSave);
            }

            return playerSaveList;
        }
        
        [Authorize("CanAccessSelfInfo")]
        [HttpDelete("{saveId}/delete")]
        public async Task<ActionResult> Delete(string userId, string saveId) {
            var user = await _userProvider.Get(userId);
            if(user == null) return NotFound();
            if(!user.Saves.ContainsKey(saveId)) return NotFound(saveId);
            user.Saves.Remove(saveId);
            await _userProvider.Save(user);

            return Ok(user);
        }

        [Authorize("CanAccessSelfInfo")]
        [HttpGet]
        public async Task<List<string>> GetAllSaves(string userId) {
            var user = await _userProvider.Get(userId);
            List<string> saveList = new List<string>();
            if(user == null) return saveList;

            JsonSerializerOptions policy = new JsonSerializerOptions();
            policy.PropertyNameCaseInsensitive = true;

            foreach(var savePair in user.Saves) {
                string gameSave = JsonSerializer.Serialize<GameSave>(savePair.Value.GameSave, policy);
                saveList.Add(gameSave);
            }

            return saveList;
        }
    }
}